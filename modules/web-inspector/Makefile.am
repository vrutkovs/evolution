module_LTLIBRARIES = module-web-inspector.la

module_web_inspector_la_CPPFLAGS = 			\
	$(AM_CPPFLAGS)					\
	-I$(top_srcdir)					\
	-DG_LOG_DOMAIN=\"evolution-web-inspector\"	\
	$(EVOLUTION_DATA_SERVER_CFLAGS)			\
	$(GNOME_PLATFORM_CFLAGS)			\
	$(CODE_COVERAGE_CFLAGS)				\
	$(NULL)

module_web_inspector_la_SOURCES = \
	evolution-web-inspector.c

module_web_inspector_la_LIBADD = \
	$(top_builddir)/e-util/libevolution-util.la	\
	$(EVOLUTION_DATA_SERVER_LIBS)			\
	$(GNOME_PLATFORM_LIBS)				\
	$(NULL)

module_web_inspector_la_LDFLAGS = \
	-module -avoid-version $(NO_UNDEFINED) $(CODE_COVERAGE_LDFLAGS)

-include $(top_srcdir)/git.mk
