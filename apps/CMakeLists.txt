set(APPLIST
  animationView
  gameJam
  sceneDecomposition
  menu_test
)

foreach(APP ${APPLIST})
  add_subdirectory(${APP})  

endforeach(APP ${APPLIST})

set(APPLIST ${APPLIST} PARENT_SCOPE)
