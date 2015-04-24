# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/testcsdl

out_testcsdl_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))	\
	$(EXPAT_CFLAGS)

out_testcsdl_LDADD = out/libodatabase.la out/libodata.la

out_testcsdl_SOURCES = \
    src/tests/csdl/testcsdl.c

nodist_out_testcsdl_SOURCES = \
    src/RedfishSchema/AccountService.c \
    src/RedfishSchema/Chassis.c \
    src/RedfishSchema/ComputerSystem.c \
    src/RedfishSchema/DMTFExtensions.c \
    src/RedfishSchema/Errors.c \
    src/RedfishSchema/EthernetNetworkInterface.c \
    src/RedfishSchema/Event.c \
    src/RedfishSchema/EventService.c \
    src/RedfishSchema/Firmware.c \
    src/RedfishSchema/IPAddresses.c \
    src/RedfishSchema/ListenerDestination.c \
    src/RedfishSchema/LogEntry.c \
    src/RedfishSchema/LogService.c \
    src/RedfishSchema/ManagerAccount.c \
    src/RedfishSchema/Manager.c \
    src/RedfishSchema/ManagerNetworkService.c \
    src/RedfishSchema/Messages.c \
    src/RedfishSchema/MiscMetrics.c \
    src/RedfishSchema/PowerMetrics.c \
    src/RedfishSchema/Redundancy.c \
    src/RedfishSchema/Resource.c \
    src/RedfishSchema/SerialInterface.c \
    src/RedfishSchema/ServiceRoot.c \
    src/RedfishSchema/Session.c \
    src/RedfishSchema/SimpleStorage.c \
    src/RedfishSchema/Task.c \
    src/RedfishSchema/TaskService.c \
    src/RedfishSchema/ThermalMetrics.c \
    src/RedfishSchema/VirtualMedia.c

DISTCLEANFILES+= $(nodist_out_testcsdl_SOURCES)

EXTRA_DIST += src/RedfishSchema/Chipwise src/RedfishSchema/Contoso src/RedfishSchema/Service

EXTRA_DIST += 	\
	src/RedfishSchema/AccountService.0.94.0.metadata	\
	src/RedfishSchema/Chassis.0.94.0.metadata	\
	src/RedfishSchema/Chipwise/Chipwise.CustomTypes.metadata	\
	src/RedfishSchema/ComputerSystem.0.94.0.metadata	\
	src/RedfishSchema/Contoso/Contoso.CustomTypes.metadata	\
	src/RedfishSchema/DMTFExtensions.0.94.0.metadata	\
	src/RedfishSchema/Errors.0.94.0.metadata	\
	src/RedfishSchema/EthernetNetworkInterface.0.94.0.metadata	\
	src/RedfishSchema/Event.0.94.0.metadata	\
	src/RedfishSchema/EventService.0.94.0.metadata	\
	src/RedfishSchema/Firmware.0.94.0.metadata	\
	src/RedfishSchema/IPAddresses.0.94.0.metadata	\
	src/RedfishSchema/ListenerDestination.0.94.0.metadata	\
	src/RedfishSchema/LogEntry.0.94.0.metadata	\
	src/RedfishSchema/LogService.0.94.0.metadata	\
	src/RedfishSchema/Manager.0.94.0.metadata	\
	src/RedfishSchema/ManagerAccount.0.94.0.metadata	\
	src/RedfishSchema/ManagerNetworkService.0.94.0.metadata	\
	src/RedfishSchema/Messages.0.94.0.metadata	\
	src/RedfishSchema/MiscMetrics.0.94.0.metadata	\
	src/RedfishSchema/PowerMetrics.0.94.0.metadata	\
	src/RedfishSchema/Redundancy.0.94.0.metadata	\
	src/RedfishSchema/Resource.0.94.0.metadata	\
	src/RedfishSchema/SerialInterface.0.94.0.metadata	\
	src/RedfishSchema/ServiceRoot.0.94.0.metadata	\
	src/RedfishSchema/Service/Service.Annotations.metadata	\
	src/RedfishSchema/Service/Service.metadata	\
	src/RedfishSchema/Session.0.94.0.metadata	\
	src/RedfishSchema/SimpleStorage.0.94.0.metadata	\
	src/RedfishSchema/Task.0.94.0.metadata	\
	src/RedfishSchema/TaskService.0.94.0.metadata	\
	src/RedfishSchema/ThermalMetrics.0.94.0.metadata	\
	src/RedfishSchema/VirtualMedia.0.94.0.metadata
