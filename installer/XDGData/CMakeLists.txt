include(GNUInstallDirs)

configure_file(
    com.github.weimens.wifiheatmap.appdata.xml.in
    ${CMAKE_BINARY_DIR}/com.github.weimens.wifiheatmap.appdata.xml
)
install(
    FILES ${CMAKE_BINARY_DIR}/com.github.weimens.wifiheatmap.appdata.xml
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
)

install(
    FILES com.github.weimens.wifiheatmap.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

install(
    FILES com.github.weimens.wifiheatmap.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
)

install(
    FILES com.github.weimens.wifiheatmap-file.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
)

install(
    FILES com.github.weimens.wifiheatmap.xml
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages
)
