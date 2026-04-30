include(FetchContent)

if(NOT TARGET nlohmann_json)
    FetchContent_Declare(
        json 
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()