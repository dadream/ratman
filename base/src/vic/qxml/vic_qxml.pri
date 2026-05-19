vic_qxml {
  vic_check_include(vic/qxml/database.hpp) {}
  vic_check_lib(*vic_qxml.*) {}

  LIBS += -lvic_qxml

  message(Configured for vic_qxml)
}
