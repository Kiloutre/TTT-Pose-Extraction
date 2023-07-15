# CMakeList.txt : projet CMake pour TTT_AnimExtraction, incluez la source et définissez
# la logique spécifique au projet ici.
#

# Ajoutez une source à l'exécutable de ce projet.
add_executable (TTT_AnimExtraction "TTT_AnimExtraction.cpp" "Utils.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TTT_AnimExtraction PROPERTY CXX_STANDARD 20)
endif()

# TODO: Ajoutez des tests et installez des cibles si nécessaire.
