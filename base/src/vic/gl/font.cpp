//+++HDR+++
//======================================================================
//   This file is part of the RATMAN software framework.
//   Copyright (C) 2009 by CRS4, Pula, Italy.
//
//   For more information, visit the CRS4 Visual Computing Group
//   web pages at http://www.crs4.it/vic/
//
//   This file may be used under the terms of the GNU General Public
//   License as published by the Free Software Foundation and appearing
//   in the file LICENSE included in the packaging of this file.
//
//   CRS4 reserves all rights not expressly granted herein.
//  
//   This file is provided AS IS with NO WARRANTY OF ANY KIND, 
//   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS 
//   FOR A PARTICULAR PURPOSE.
//
//======================================================================
//---HDR---//
#ifdef _WIN32
#include <windows.h>
#endif

#include <vic/gl/font.hpp>
#include <vic/gl/gl.hpp>
#include <iostream>

namespace vic {
  
  namespace gl {
    
    // ------------------------------------------------------------------------------------------------
    // Embedded default font - defines struct default_font_luminance
    // ------------------------------------------------------------------------------------------------
#include "arial16-csrc.c"
    
    // ------------------------------------------------------------------------------------------------
    // Font shadowing+embossing
    // ------------------------------------------------------------------------------------------------

    static inline GLubyte atop(GLubyte y, GLubyte alpha, GLubyte x) {
      float a_f = alpha/255.0f;
      float y_f = y;
      float x_f = x;
      float r_f = a_f * x_f + (1.0f-a_f)*y_f;
      int r_i = (int)(r_f+0.5f);
      return GLubyte((r_i<0)?0:((r_i>255)?255:r_i));
    }
    
    // ------------------------------------------------------------------------------------------------
    // Class implementation
    // ------------------------------------------------------------------------------------------------

    void font::construct_from_one_channel(const unsigned char* idata,
                                          int sz,
                                          bool proportional,
                                          bool shadowed,
                                          bool embossed) {
      if (texture_data_) delete [] texture_data_;
      texture_data_ = 0;
      
      cleanup();
      
      texture_dx_ = sz;
      texture_dy_ = sz;
      character_dx_ = sz/16;
      character_dy_ = sz/16;
      character_fixed_width_  = float(character_dx_)/float(texture_dx_);
      character_fixed_height_ = float(character_dy_)/float(texture_dy_);
      is_proportional_ = proportional;
      scale_x_ = character_dx_; // 1-1 pixel mapping
      scale_y_ = character_dy_;
      spacing_ = (is_proportional_ ? (0.05f) : (0.0f));

      reset_texture_data(idata,shadowed,embossed);
      reset_glyphs();
    }
    
    font::font(const unsigned char* idata,
               int sz,
               bool proportional) {
      texture_data_ = 0;
      texture_id_ = 0;
      begin_depth_ = 0;
      construct_from_one_channel(idata, sz, proportional,true,true);
    }

    font::font() {
      texture_data_ = 0;
      texture_id_ = 0;
      begin_depth_ = 0;

      construct_from_one_channel(default_font_luminance.pixel_data,
                                 default_font_luminance.width,
                                 true,
                                 true,
                                 true); 
    }
  
    font::~font() {
      if (texture_data_) {
        delete[] texture_data_; texture_data_ = 0;
      }
      cleanup();
    }
    
#define L_IDX(x,y,w)     ((y)*(w)+(x))
#define LA_IDX(x,y,ia,w) (((y)*(w)+(x))*2+(ia))

    void font::reset_texture_data(const unsigned char* idata, bool shadowed, bool embossed) {
      // For now, just copy luminance to alpha
      if (texture_data_) delete [] texture_data_;
      texture_data_ = new unsigned char [ texture_dx_ * texture_dy_ * 2 ];

      // Background
      for (int x=0; x<texture_dx_; ++x) {
        for (int y=0; y<texture_dy_; ++y) {
          GLubyte luminance = 0;
          GLubyte alpha = 0;
          
          texture_data_[LA_IDX(x,y,0,texture_dx_)] = luminance;
          texture_data_[LA_IDX(x,y,1,texture_dx_)] = alpha;
        }
      }

      // Shadows
      {
        bool shadow_status[2] = { shadowed, embossed };
        int  shadow_dx[2] = { -1, 1};
        int  shadow_dy[2] = { -1, 1};

        for (int shadow = 0; shadow<2; ++shadow) {
          if (shadow_status[shadow]) {
            for (int x=0; x<texture_dx_; ++x) {
              for (int y=0; y<texture_dy_; ++y) {
                GLubyte luminance = 0; // black
                GLubyte alpha = idata[L_IDX(x,y,texture_dx_)];

                int x1 = x + shadow_dx[shadow];
                int y1 = y + shadow_dy[shadow];

                if ((x1>=0) && (x1<texture_dx_)&&
                    (y1>=0) && (y1<texture_dy_)) {
                  texture_data_[LA_IDX(x1,y1,0,texture_dx_)] = atop(texture_data_[LA_IDX(x1,y1,0,texture_dx_)], alpha, luminance);
                  texture_data_[LA_IDX(x1,y1,1,texture_dx_)] = atop(texture_data_[LA_IDX(x1,y1,1,texture_dx_)], alpha, alpha);
                }
              }
            }
          }
        }
      }
      
      // Font glyphs
      for (int x=0; x<texture_dx_; ++x) {
        for (int y=0; y<texture_dy_; ++y) {
          GLubyte alpha = idata[L_IDX(x,y,texture_dx_)];
          GLubyte luminance = 255; // white
          
          texture_data_[LA_IDX(x,y,0,texture_dx_)] = atop(texture_data_[LA_IDX(x,y,0,texture_dx_)], alpha, luminance);
          texture_data_[LA_IDX(x,y,1,texture_dx_)] = atop(texture_data_[LA_IDX(x,y,1,texture_dx_)], alpha, alpha);
        }
      }

      // Grid: make sure tiles remain separated
      for (int x=character_dx_; x<texture_dx_; x+=character_dx_) {
        for (int y=0; y<texture_dy_; ++y) {
          GLubyte alpha = 0;
          GLubyte luminance = 0;
          texture_data_[LA_IDX(x,y,0,texture_dx_)] = luminance;
          texture_data_[LA_IDX(x,y,1,texture_dx_)] = alpha;
        }
      }
      for (int y=character_dy_; y<texture_dy_; y+=character_dy_) {
        for (int x=0; x<texture_dx_; ++x) {
          GLubyte alpha = 0;
          GLubyte luminance = 0;
          texture_data_[LA_IDX(x,y,0,texture_dx_)] = luminance;
          texture_data_[LA_IDX(x,y,1,texture_dx_)] = alpha;
        }
      }
    }
  
    void font::reset_glyphs() {
      int cx = texture_dx_ / character_dx_;
      //int cy = texture_dy_ / character_dy_;
      int dx = texture_dx_;
      
      /* rebuild the character information table */
      for (int i = 0; i < 256; i++) {
        int x = i % cx;
        int y = i / cx;
        x *= character_dx_;
        y *= character_dy_;
        
        if (!is_proportional()) {
          glyphs_[i] = font_glyph(float(x)/texture_dx_,
                                  float(y)/texture_dy_,
                                  1.0f, 
                                  0.0f);
        } else {
          // Find left margin from alpha channel
          int b=0;
          for (int s = 0; b < character_dx_ && s == 0; ++b) {
            for (int a = 0; a < character_dy_; a++) {
              if (texture_data_[LA_IDX(x+b,y+a,1,dx)] != 0) {
                s = 1;
                break;
              }
            }
          }
          int lm = b-1;

          // find the right margin from alpha channel
          b = character_dx_-1;
          for (int s = 0; b >= 0 && s == 0; b--) {
            for (int a = 0; a < character_dy_; a++) {
              if (texture_data_[LA_IDX(x+b,y+a,1,dx)]) {
                s = 1;
                break;
              }
            }
          }
          int rm = b+1;
          
          if (lm >= rm) {
            lm = 0;
            rm = character_dx_>>1;
          }
          
          float width = float(rm-lm+1)/float(character_dx_);
          float left_margin = float(lm)/float(character_dx_);
          if (width<0.0f) width = 0.0;
          if (width>1.0f) width = 1.0f;
          if (left_margin<0.0f) left_margin = 0.0f;
          if (left_margin>1.0f) left_margin = 1.0f;
          glyphs_[i] = font_glyph(float(x)/texture_dx_,
                                  float(y)/texture_dy_,
                                  width,
                                  left_margin);

        }
#if 0
        std::cerr << (is_proportional() ? "PROP." : "FIXED");
        std::cerr << " GLYPH[" << i << "] => (" <<
          glyphs_[i].u() << ", " << 
          glyphs_[i].v() << ", " << 
          glyphs_[i].width() << ", " << 
          glyphs_[i].left_margin() << ")" <<
          std::endl;
#endif
      }
    }

    void font::set_spacing(float spacing) {
      spacing_ = spacing;
    }
    
    void font:: set_scaling(float sx, float sy) {
      scale_x_ = sx;
      scale_y_ = sy;
    }
  
    void font::set_proportional(bool b) {
      if (is_proportional_ != b) {
        is_proportional_ = b;
        reset_glyphs();
      }
    }

    float font::string_width(const char* s) const {
      float result = 0.0f;
      if (!is_proportional_) {
        const float fx = scale_x_ + (1.0f+spacing_);
        for (int n=0; s[n]!=0; ++n) {
          result += fx;
        }
        result -= scale_x_*spacing_; // remove space after last char
      } else {
        float spx = spacing_;
        for (int n=0; s[n]!=0; ++n) {
	  unsigned u_n = (unsigned char)(s[n]);
          result += scale_x_ * (glyphs_[u_n].width() + spx);
        }
        result -= scale_x_*spacing_; // remove space after last char
      }
      return result;    
    }

    void font::string_bbox(const char* s, float& x0, float&y0, float&x1, float& y1) const {
      x0 = 0.0;
      x1 = x0 + string_width(s);
      y0 = 0.0;
      y1 = y0 + scale_y_;
    }
    
    void font::begin() const {
      if (begin_depth_ == 0) {
        if (texture_id_ == 0) {
          glGenTextures(1, &texture_id_);
          glBindTexture(GL_TEXTURE_2D, texture_id_);

          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
          
          gluBuild2DMipmaps(GL_TEXTURE_2D,
                            GL_LUMINANCE_ALPHA,
                            texture_dx_, texture_dy_,
                            GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                            texture_data_);
          //std::cerr << "Created texture" << texture_id_ << std::endl;
        } else {
          glBindTexture(GL_TEXTURE_2D, texture_id_);
          //std::cerr << "Bound texture" << texture_id_ << std::endl;
        }
        glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT );
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      ++begin_depth_;
    }
    
    void font::end() const {
      --begin_depth_;
      if (begin_depth_ == 0) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glPopAttrib();
        //std::cerr << "Unbound texture" << texture_id_ << std::endl;
      }
    }
    
    void font::cleanup() const {
      if (texture_id_) {
        glDeleteTextures(1, &texture_id_);
        texture_id_ = 0;
      }
    }
  
    void font::write(const char* s) const {
      begin();
      {
        const float uconv = float(character_dx_)/float(texture_dx_);
        const float vconv = float(character_dy_)/float(texture_dy_);
        const float fx = character_fixed_width_;
        const float fy = character_fixed_height_;
        const float y0 = 0.0f;
        const float y1 = scale_y_;
        const float sx = scale_x_;
        const float spx = spacing_;
        float x0 = 0.0f;
      
        glBegin(GL_QUADS);
        {
          for (const unsigned char *u = (const unsigned char *)s;
               *u;
               ++u) {
            const font_glyph& c = glyphs_[*u];
            
            // Render glyph
            float x1 = x0+c.width()*sx;
            float u0 = c.u() + c.left_margin()*uconv;
            float u1 = u0+c.width()*uconv;
            float v0 = c.v();
            float v1 = v0+fy;
            glTexCoord2f(u0,v1); glVertex2f(x0, y0); 
            glTexCoord2f(u1,v1); glVertex2f(x1, y0);
            glTexCoord2f(u1,v0); glVertex2f(x1, y1);
            glTexCoord2f(u0,v0); glVertex2f(x0, y1);
            
            // Add extra spacing
            x0 = x1 + spx*sx;
          }
        }
        glEnd();
      }
      end();
    }
  } // namespace gl
} // namespace vic
