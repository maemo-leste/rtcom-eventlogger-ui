INCLUDES = \
	$(RTCOM_EVENTLOGGER_UI_CFLAGS) \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"

lib_LTLIBRARIES = librtcom-eventlogger-ui.la

librtcom_eventlogger_ui_includefir = $(includedir)/rtcom-eventlogger-ui
librtcom_eventlogger_ui_la_SOURCES = \
	rtcom-log-view.c \
	rtcom-log-model.c \
	rtcom-log-search-bar.c \
	utf8.c
librtcom_eventlogger_ui_la_LIBADD = $(RTCOM_EVENTLOGGER_UI_LIBS)

librtcom_eventlogger_ui_la_LDFLAGS = -no-undefined \
        -export-symbols-regex "^rtcom_log_.*" \
        -version-info "$(LT_CURRENT)":"$(LT_REVISION)":"$(LT_AGE)"


