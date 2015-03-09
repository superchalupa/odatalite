# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/testcsdl

out_testcsdl_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes))	\
	$(EXPAT_CFLAGS)

out_testcsdl_LDADD = out/libbase.la out/libodata.la

out_testcsdl_SOURCES = \
    src/tests/csdl/testcsdl.c \
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

