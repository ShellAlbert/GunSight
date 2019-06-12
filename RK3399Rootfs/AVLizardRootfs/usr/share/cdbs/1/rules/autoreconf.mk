# autoreconf.mk - dh-autoreconf integration for CDBS.
include /usr/share/cdbs/1/rules/buildcore.mk

CDBS_BUILD_DEPENDS_rules_autoreconf := dh-autoreconf
CDBS_BUILD_DEPENDS += , $(CDBS_BUILD_DEPENDS_rules_autoreconf)

post-patches:: debian/autoreconf.after

debian/autoreconf.after:
	dh_autoreconf $(DEB_DH_AUTORECONF_ARGS)

reverse-config::
	if test -e debian/autoreconf.before; then \
		dh_autoreconf_clean $(DEB_DH_AUTORECONF_CLEAN_ARGS); \
	fi
