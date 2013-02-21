config_files = ip.conf 
daemon_files = slurm-serverd

EXTRA_DIST = $(config_files) $(daemon_files)

install-data-local:
	##install config_files
	@ p="$(config_files)"; \
	for file in $$p; do \
	  if test -f $(DESTDIR)$(sysconfdir)/$$file; then \
	    echo "******************************* WARNING ************************************"; \
	    echo "*** Not installing new $$file over existing file in:"; \
	    echo "***   $(DESTDIR)$(sysconfdir)/$$file"; \
	    echo "******************************* WARNING ************************************"; \
	  else \
	    if test -f "$$file"; then d=; else d="$(srcdir)/"; fi; \
	    f="`echo $$file | sed -e 's|^.*/||'`"; \
	    echo "$(INSTALL_DATA) $$d$$file $(DESTDIR)$(sysconfdir)/$$f"; \
	    $(INSTALL_DATA) $$d$$file $(DESTDIR)$(sysconfdir)/$$f; \
	  fi; \
	done
	##install daemon_files
	@ p="$(daemon_files)"; \
	for file in $$p; do \
	  if test -f $(DESTDIR)/etc/init.d/$$file; then \
	    echo "******************************* WARNING ************************************"; \
	    echo "*** Not installing new $$file over existing file in:"; \
	    echo "***   $(DESTDIR)/etc/init.d/$$file"; \
	    echo "******************************* WARNING ************************************"; \
	  else \
	    if test -f "$$file"; then d=; else d="$(srcdir)/"; fi; \
	    f="`echo $$file | sed -e 's|^.*/||'`"; \
	    echo "$(INSTALL_DATA) $$d$$file $(DESTDIR)/etc/init.d/$$f"; \
	    $(INSTALL_DATA) $$d$$file $(DESTDIR)/etc/init.d/$$f; \
	    chmod +x $(DESTDIR)/etc/init.d/$$f; \
	  fi; \
	done

# Only remove if exactly the same as what in our tree
# NOTE TO READER: Bourne shell if ... fi evaluates the body if 
#    the return of the evaluted command is 0 (as opposed to non-zero
#    as used by everyone else)
uninstall-local:
	##uninstall config_files
	@ p="$(config_files)"; \
	for file in $$p; do \
	  if test -f "$(DESTDIR)$(sysconfdir)/$$file"; then \
	  	if test -f "$$file"; then d=; else d="$(srcdir)/"; fi; \
	    if diff "$(DESTDIR)$(sysconfdir)/$$file" "$$d$$file" > /dev/null 2>&1 ; then \
	      echo "rm -f $(DESTDIR)$(sysconfdir)/$$file" ; \
	      rm -f "$(DESTDIR)$(sysconfdir)/$$file" ; \
	    fi ; \
	  fi ; \
	done
	##uninstall daemon_files
	@ p="$(daemon_files)"; \
	for file in $$p; do \
	  if test -f "$(DESTDIR)/etc/init.d/$$file"; then \
	  	if test -f "$$file"; then d=; else d="$(srcdir)/"; fi; \
	    if diff "$(DESTDIR)/etc/init.d/$$file" "$$d$$file" > /dev/null 2>&1 ; then \
	      echo "rm -f $(DESTDIR)/etc/init.d/$$file" ; \
	      rm -f "$(DESTDIR)/etc/init.d/$$file" ; \
	    fi ; \
	  fi ; \
	done
	
##	@ p="$(daemon_files)"; \
##	for file in $$p; do \
##	  if test -f "$(DESTDIR)/etc/init.d/$$file"; then \
##	  	if test -f "$$file"; then d=; else d="$(srcdir)/"; fi; \
##	    if diff "$(DESTDIR)/etc/init.d/$$file" "$$d$$file" > /dev/null 2>&1 ; then \
##	      echo "rm -f $(DESTDIR)/etc/init.d/$$file" ; \
##	      rm -f "$(DESTDIR)/etc/init.d/$$file" ; \
##	    fi ; \
##	  fi ; \
##	done