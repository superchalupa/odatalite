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
    src/RedfishSchema/EthernetInterface.c \
    src/RedfishSchema/Event.c \
    src/RedfishSchema/EventService.c \
    src/RedfishSchema/IPAddresses.c \
    src/RedfishSchema/JsonSchemaFile.c \
    src/RedfishSchema/ListenerDestination.c \
    src/RedfishSchema/LogEntry.c \
    src/RedfishSchema/LogService.c \
    src/RedfishSchema/Manager.c \
    src/RedfishSchema/ManagerAccount.c \
    src/RedfishSchema/ManagerNetworkProtocol.c \
    src/RedfishSchema/Messages.c \
    src/RedfishSchema/Power.c \
    src/RedfishSchema/Privileges.c \
    src/RedfishSchema/Processor.c \
    src/RedfishSchema/Redundancy.c \
    src/RedfishSchema/Resource.c \
    src/RedfishSchema/Role.c \
    src/RedfishSchema/SerialInterface.c \
    src/RedfishSchema/ServiceRoot.c \
    src/RedfishSchema/Session.c \
    src/RedfishSchema/SessionService.c \
    src/RedfishSchema/SimpleStorage.c \
    src/RedfishSchema/Task.c \
    src/RedfishSchema/TaskService.c \
    src/RedfishSchema/Thermal.c \
    src/RedfishSchema/VirtualMedia.c \
    src/RedfishSchema/VLanNetworkInterface.c

DISTCLEANFILES+= $(nodist_out_testcsdl_SOURCES)

EXTRA_DIST += 	\
	src/RedfishSchema/AccountService.0.96.0.metadata \
	src/RedfishSchema/Chassis.0.96.0.metadata \
	src/RedfishSchema/ComputerSystem.0.96.0.metadata \
	src/RedfishSchema/DMTFExtensions.0.96.0.metadata \
	src/RedfishSchema/EthernetInterface.0.96.0.metadata \
	src/RedfishSchema/Event.0.96.0.metadata \
	src/RedfishSchema/EventService.0.96.0.metadata \
	src/RedfishSchema/IPAddresses.0.96.0.metadata \
	src/RedfishSchema/JsonSchemaFile.0.96.0.metadata \
	src/RedfishSchema/ListenerDestination.0.96.0.metadata \
	src/RedfishSchema/LogEntry.0.96.0.metadata \
	src/RedfishSchema/LogService.0.96.0.metadata \
	src/RedfishSchema/Manager.0.96.0.metadata \
	src/RedfishSchema/ManagerAccount.0.96.0.metadata \
	src/RedfishSchema/ManagerNetworkProtocol.0.96.0.metadata \
	src/RedfishSchema/Messages.0.96.0.metadata \
	src/RedfishSchema/Power.0.96.0.metadata \
	src/RedfishSchema/Privileges.0.96.0.metadata \
	src/RedfishSchema/Processor.0.96.0.metadata \
	src/RedfishSchema/Redundancy.0.96.0.metadata \
	src/RedfishSchema/Resource.0.96.0.metadata \
	src/RedfishSchema/Role.0.96.0.metadata \
	src/RedfishSchema/SerialInterface.0.96.0.metadata \
	src/RedfishSchema/ServiceRoot.0.96.0.metadata \
	src/RedfishSchema/Session.0.96.0.metadata \
	src/RedfishSchema/SessionService.0.96.0.metadata \
	src/RedfishSchema/SimpleStorage.0.96.0.metadata \
	src/RedfishSchema/Task.0.96.0.metadata \
	src/RedfishSchema/TaskService.0.96.0.metadata \
	src/RedfishSchema/Thermal.0.96.0.metadata \
	src/RedfishSchema/VirtualMedia.0.96.0.metadata \
	src/RedfishSchema/VLanNetworkInterface.0.96.0.metadata
