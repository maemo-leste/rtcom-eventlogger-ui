## Process this file with automake to produce Makefile.in

SUBDIRS = \
	src \
	example

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = rtcom-eventlogger-ui.pc

librtcom_eventlogger_ui_docdir = ${prefix}/doc/rtcom-eventlogger-ui
librtcom_eventlogger_ui_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS

nobase_include_HEADERS = \
	rtcom-eventlogger-ui/rtcom-log-model.h \
	rtcom-eventlogger-ui/rtcom-log-view.h \
	rtcom-eventlogger-ui/rtcom-log-columns.h \
	rtcom-eventlogger-ui/rtcom-log-search-bar.h

EXTRA_DIST = \
	$(librtcom_eventlogger_ui_doc_DATA)

