# Copyright (c) 2003-2004 E. Will et al.
# Rights to this code are documented in doc/LICENSE.
#
# This file contains build instructions.
#
# $Id: Makefile.in 8375 2007-06-03 20:03:26Z pippijn $
#

MODULE = chanserv

SRCS =			\
	main.c	\
	aspircdquiet.c   \
	access.c \
	akick.c	\
	antiflood.c	\
	ban.c		\
	clear.c	\
	clear_akicks.c  \
	clear_bans.c	\
	clear_flags.c	\
	clear_ops.c	\
	clear_users.c	\
	clone.c	\
	close.c	\
	count.c	\
	drop.c	\
	fflags.c	\
	flags.c	\
	freeze.c	\
	fsuspend.c	\
	ftransfer.c	\
	getkey.c	\
	halfop.c	\
	help.c	\
	hold.c	\
	info.c	\
	invite.c	\
	kick.c	\
	list.c	\
	mark.c	\
	mass.c	\
	mass_halfop.c	\
	mass_op.c	\
	mass_owner.c	\
	mass_protect.c	\
	mass_voice.c	\
	moderate.c \
	op.c		\
	owner.c	\
	protect.c	\
	quiet.c	\
	recover.c	\
	register.c	\
	set.c		\
	set_core.c	\
	set_email.c	\
	set_entrymsg.c	\
	set_fantasy.c	\
	set_founder.c	\
	set_gameserv.c \
	set_guard.c	\
	set_keeptopic.c	\
	set_limitflags.c \
	set_mlock.c	\
	set_noop.c	\
	set_prefix.c	\
	set_private.c	\
	set_property.c	\
	set_pubacl.c	\
	set_restricted.c  \
	set_secure.c	\
	set_topiclock.c	\
	set_url.c	\
	set_verbose.c	\
	successor_acl.c \
	status.c	\
	suspend.c	\
	sync.c		\
	aspircdquiet.c   \
	taxonomy.c	\
	template.c	\
	topic.c	\
	unban_self.c	\
	version.c	\
	voice.c	\
	why.c		\
	xop.c

include ../../extra.mk
include ../../buildsys.mk
include ../../buildsys.module.mk

CPPFLAGS	+= -I../../include
LIBS +=	-L../../libathemecore -lathemecore ${LDFLAGS_RPATH}
