/*
 * Copyright (c) 2003-2004 E. Will et al.
 * Copyright (c) 2005-2007 Atheme Development Group
 * Rights to this code are documented in doc/LICENSE.
 *
 * This file contains protocol support for ircd-seven.
 *
 */

#include "atheme.h"
#include "uplink.h"
#include "pmodule.h"
#include "protocol/elemental-ircd.h"

DECLARE_MODULE_V1("protocol/aspIRCd", true, _modinit, NULL, PACKAGE_STRING, "Shaltúre developers <https://github.com/shalture>");

/* *INDENT-OFF* */

ircd_t Chatd = {
	.ircdname = "aspIRCd",
	.tldprefix = "$$",
	.uses_uid = true,
	.uses_rcommand = false,
	.uses_owner = true,
	.uses_protect = true,
	.uses_halfops = true,
	.uses_p10 = false,
	.uses_vhost = false,
	.oper_only_modes = CMODE_EXLIMIT | CMODE_PERM | CMODE_IMMUNE,
	.owner_mode = 0,
	.protect_mode = 0,
	.halfops_mode = 0,
	.owner_mchar = "+q",
	.protect_mchar = "+a",
	.halfops_mchar = "+h",
	.type = PROTOCOL_CHARYBDIS,
	.perm_mode = CMODE_PERM,
	.oimmune_mode = CMODE_IMMUNE,
	.ban_like_modes = "beIy",
	.except_mchar = 'e',
	.invex_mchar = 'I',
	.flags = IRCD_CIDR_BANS | IRCD_HOLDNICK | IRCD_TOPIC_NOCOLOUR,
};

struct cmode_ chatd_mode_list[] = {
  { 'i', CMODE_INVITE },
  { 'm', CMODE_MOD    },
  { 'n', CMODE_NOEXT  },
  { 'p', CMODE_PRIV   },
  { 's', CMODE_SEC    },
  { 't', CMODE_TOPIC  },
  { 'c', CMODE_NOCOLOR},
  { 'R', CMODE_REGONLY},
  { 'r', CMODE_CHANREG},
  { 'z', CMODE_OPMOD  },
  { 'g', CMODE_FINVITE},
  { 'L', CMODE_EXLIMIT},
  { 'P', CMODE_PERM   },
  { 'F', CMODE_FTARGET},
  { 'Q', CMODE_DISFWD },
  { 'M', CMODE_IMMUNE },
  { 'T', CMODE_NONOTICE},
  { 'C', CMODE_NOCTCP },

  /* following modes are added as extensions */
  { 'S', CMODE_SSLONLY	 },
  { 'O', CMODE_OPERONLY  },
  { 'A', CMODE_ADMINONLY },

  { '\0', 0 }
};

struct cmode_ chatd_status_mode_list[] = {
  { 'B', CSTATUS_IMMUNE	 },
  { 'q', CSTATUS_OWNER	 },
  { 'a', CSTATUS_PROTECT },
  { 'o', CSTATUS_OP	 },
  { 'h', CSTATUS_HALFOP  },
  { 'v', CSTATUS_VOICE	 },
  { '\0', 0 }
};

struct cmode_ chatd_prefix_mode_list[] = {
  { '!', CSTATUS_IMMUNE	 },
  { '~', CSTATUS_OWNER	 },
  { '&', CSTATUS_PROTECT },
  { '@', CSTATUS_OP	 },
  { '%', CSTATUS_HALFOP  },
  { '+', CSTATUS_VOICE	 },
  { '\0', 0 }
};


struct cmode_ chatd_user_mode_list[] = {
  { 'p', UF_IMMUNE   },
  { 'a', UF_ADMIN    },
  { 'i', UF_INVIS    },
  { 'o', UF_IRCOP    },
  { 'D', UF_DEAF     },
  { 'S', UF_SERVICE  },
  { '\0', 0 }
};

/* *INDENT-ON* */

static bool chatd_is_valid_hostslash(const char *host)
{
	const char *p;
	bool dot = false;

	if (*host == '.' || *host == '/' || *host == ':')
		return false;

	for (p = host; *p != '\0'; p++)
	{
		if (*p == '.' || *p == ':' || *p == '/')
			dot = true;
		else if (!((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'Z') ||
					(*p >= 'a' && *p <= 'z') || *p == '-'))
			return false;
	}
	/* hyperion allows a trailing / but RichiH does not want it, whatever */
	if (dot && p[-1] == '/')
		return false;
	return dot;
}

static void chatd_wallops_sts(const char *reason)
{
	sts(":%s ENCAP * SNOTE s :%s", ME, reason);
}

/* The following m_functions are copied from generic_ts6, with additions to handle the
 * "identified" / "owns this nick" flag.
 */

static void m_euid(sourceinfo_t *si, int parc, char *parv[])
{
	server_t *s;
	user_t *u;

	/* got the right number of args for an introduction? */
	if (parc >= 11)
	{
		s = si->s;
		slog(LG_DEBUG, "m_euid(): new user on `%s': %s", s->name, parv[0]);

		u = user_add(parv[0],				/* nick */
			parv[4],				/* user */
			*parv[8] != '*' ? parv[8] : parv[5],	/* hostname */
			parv[5],				/* hostname (visible) */
			parv[6],				/* ip */
			parv[7],				/* uid */
			parv[parc - 1],				/* gecos */
			s,					/* object parent (server) */
			atoi(parv[2]));				/* hopcount */
		if (u == NULL)
			return;

		user_mode(u, parv[3]);
		if (*parv[9] != '*')
		{
			handle_burstlogin(u, parv[9], 0);
			/* If an account is given in burst, then either they logged in with sasl,
			 * or they logged in before a split and are now returning. Either way we need
			 * to check for identified-to-nick status and update the ircd state accordingly.
			 * For sasl they should be marked identified, and when returning from a split
			 * their nick may have been ungrouped, they may have changed nicks, or their account
			 * may have been dropped.
			 */
			if (authservice_loaded)
				sts(":%s ENCAP * IDENTIFIED %s %s %s", ME, CLIENT_NAME(u), u->nick,
						should_reg_umode(u) ? "" : "OFF");
		}

		/* server_eob() cannot know if a user was introduced
		 * with NICK/UID or EUID and handle_nickchange() must
		 * be called exactly once for each new user -- jilles */
		if (s->flags & SF_EOB)
			handle_nickchange(u);
	}
	else
	{
		int i;
		slog(LG_DEBUG, "m_euid(): got EUID with wrong number of params");

		for (i = 0; i < parc; i++)
			slog(LG_DEBUG, "m_euid():   parv[%d] = %s", i, parv[i]);
	}
}

static void m_nick(sourceinfo_t *si, int parc, char *parv[])
{
	server_t *s;
	user_t *u;

	/* got the right number of args for an introduction? */
	if (parc == 8)
	{
		s = server_find(parv[6]);
		if (!s)
		{
			slog(LG_DEBUG, "m_nick(): new user on nonexistant server: %s", parv[6]);
			return;
		}

		slog(LG_DEBUG, "m_nick(): new user on `%s': %s", s->name, parv[0]);

		u = user_add(parv[0], parv[4], parv[5], NULL, NULL, NULL, parv[7], s, atoi(parv[2]));
		if (u == NULL)
			return;

		user_mode(u, parv[3]);

		/* If server is not yet EOB we will do this later.
		 * This avoids useless "please identify" -- jilles */
		if (s->flags & SF_EOB)
			handle_nickchange(user_find(parv[0]));
	}

	/* if it's only 2 then it's a nickname change */
	else if (parc == 2)
	{
		bool realchange;

		if (!si->su)
		{
			slog(LG_DEBUG, "m_nick(): server trying to change nick: %s", si->s != NULL ? si->s->name : "<none>");
			return;
		}

		slog(LG_DEBUG, "m_nick(): nickname change from `%s': %s", si->su->nick, parv[0]);

		realchange = irccasecmp(si->su->nick, parv[0]);

		if (user_changenick(si->su, parv[0], atoi(parv[1])))
			return;

		/* fix up +e if necessary -- jilles */
		if (realchange && should_reg_umode(si->su))
			/* changed nick to registered one, reset +e */
			sts(":%s ENCAP * IDENTIFIED %s %s", ME, CLIENT_NAME(si->su), si->su->nick);

		/* It could happen that our PING arrived late and the
		 * server didn't acknowledge EOB yet even though it is
		 * EOB; don't send double notices in that case -- jilles */
		if (si->su->server->flags & SF_EOB)
			handle_nickchange(si->su);
	}
	else
	{
		int i;
		slog(LG_DEBUG, "m_nick(): got NICK with wrong number of params");

		for (i = 0; i < parc; i++)
			slog(LG_DEBUG, "m_nick():   parv[%d] = %s", i, parv[i]);
	}
}

/* protocol-specific stuff to do on login */
static void chatd_on_login(user_t *u, myuser_t *mu, const char *wantedhost)
{
	return_if_fail(u != NULL);

	sts(":%s ENCAP * SU %s %s", ME, CLIENT_NAME(u), entity(mu)->name);

	if (should_reg_umode(u))
		sts(":%s ENCAP * IDENTIFIED %s %s", ME, CLIENT_NAME(u), u->nick);
}

static bool chatd_on_logout(user_t *u, const char *account)
{
	return_val_if_fail(u != NULL, false);

	sts(":%s ENCAP * SU %s", ME, CLIENT_NAME(u));
	sts(":%s ENCAP * IDENTIFIED %s %s OFF", ME, CLIENT_NAME(u), u->nick);
	return false;
}

static void nick_group(hook_user_req_t *hdata)
{
	user_t *u;

	u = hdata->si->su != NULL && !irccasecmp(hdata->si->su->nick, hdata->mn->nick) ? hdata->si->su : user_find_named(hdata->mn->nick);
	if (u != NULL && should_reg_umode(u))
		sts(":%s ENCAP * IDENTIFIED %s %s", ME, CLIENT_NAME(u), u->nick);
}

static void nick_ungroup(hook_user_req_t *hdata)
{
	user_t *u;

	u = hdata->si->su != NULL && !irccasecmp(hdata->si->su->nick, hdata->mn->nick) ? hdata->si->su : user_find_named(hdata->mn->nick);
	if (u != NULL && !nicksvs.no_nick_ownership)
		sts(":%s ENCAP * IDENTIFIED %s %s OFF", ME, CLIENT_NAME(u), u->nick);
}

void _modinit(module_t * m)
{
	MODULE_TRY_REQUEST_DEPENDENCY(m, "protocol/charybdis");

	mode_list = chatd_mode_list;
	user_mode_list = chatd_user_mode_list;
	status_mode_list = chatd_status_mode_list;
	prefix_mode_list = chatd_prefix_mode_list;

	wallops_sts = &chatd_wallops_sts;
	ircd_on_login = &chatd_on_login;
	ircd_on_logout = &chatd_on_logout;
	is_valid_host = &chatd_is_valid_hostslash;

	pcommand_delete("NICK");
	pcommand_add("NICK", m_nick, 2, MSRC_USER | MSRC_SERVER);
	pcommand_delete("EUID");
	pcommand_add("EUID", m_euid, 11, MSRC_SERVER);

	ircd = &Chatd;

	hook_add_event("nick_group");
	hook_add_nick_group(nick_group);
	hook_add_event("nick_ungroup");
	hook_add_nick_ungroup(nick_ungroup);

	m->mflags = MODTYPE_CORE;

	pmodule_loaded = true;
}

/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs
 * vim:ts=8
 * vim:sw=8
 * vim:noexpandtab
 */
