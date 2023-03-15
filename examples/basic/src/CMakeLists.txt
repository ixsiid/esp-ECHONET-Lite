idf_component_register(SRCS "main.cpp"
                    PRIV_REQUIRES nvs_flash WiFiManager
                    INCLUDE_DIRS ".")

include_directories("../../../src")
