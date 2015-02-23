# Need GNUMake
# Created and maintanced by wuya
# v 1.0

.PHONY: de debug release cl clean up update ci checkin commit doc bak backup test run r re rebuild

BAKDIR=../bak

LIBPROJECT=dbf_decoder fix_asc_decoder s1240_decoder vary_asc_decoder db_link_decoder
ALLPRLJECT=$(shell /bin/ls)
OTHPROJECT=$(filter-out $(LIBPROJECT),$(ALLPRLJECT))

de debug release cl r run clean up update ci checkin commit doc test re rebuild:
	@for a in $(LIBPROJECT); do \
		if [ -d $$a -a -r $$a/makefile ]; then \
			echo ">>> Enter $$a >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"; \
			cd $$a; \
			make $(MAKECMDGOALS); \
			cd ..; \
			echo "<<< Leave $$a <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"; \
			echo ""; \
		fi \
	done
	@for a in $(OTHPROJECT); do \
		if [ -d $$a -a -r $$a/makefile ]; then \
			echo ">>> Enter $$a >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"; \
			cd $$a; \
			make $(MAKECMDGOALS); \
			cd ..; \
			echo "<<< Leave $$a <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"; \
			echo ""; \
		fi \
	done

bak backup:
	@mkdir -p $(BAKDIR)
	@tar cf - `find . -type f|grep -v obj|grep -v CVS|grep -v core|grep -v '\.d'|grep -v '\.o'` \
		| gzip -qc > ../bak/bak_`/bin/date +%Y_%m_%d_%H_%M`.tar.gz
	@echo backup file put to $(BAKDIR), backup ok.
	

