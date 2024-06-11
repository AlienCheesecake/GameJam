set(APPLIST
  animationView
  gameJam
)

foreach(APP ${APPLIST})
  add_subdirectory(${APP})  
endforeach(APP ${APPLIST})

set(APPLIST ${APPLIST} PARENT_SCOPE)
