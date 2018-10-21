# Adding this will suppress the requirement for INSTALL, et c., files.
AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4 

#---------------------------------------------------------------------
# Subdirectories with Makefiles


# SUBDIRS lists all directories that make should recurse into 
#   before processing the current directory
SUBDIRS=src

EXTRA_DIST=COPYRIGHT INSTALL

BINDISTFILES=COPYRIGHT INSTALL

DATADISTFILES=data

DOCDISTFILES=doc

# Indicate that these targets are not the names of files
.PHONY: nobin-dists all-dists snapshot-dist bin-dist snapshot-bin-dist \
	data-dist doc-dist

dist-all: dist data-dist doc-dist

bin-dist: all-am
	$(MAKE) $(AM_MAKE_FLAGS) prefix=$(prefix)/$(distdir) install-exec; \
	for file in $(BINDISTFILES) ; do \
	  cp -pR $$file $(distdir)/ ; \
	done; \
	rm -rf `find $(distdir) -name CVS`; \
	$(AMTAR) chof - $(distdir) | GZIP=$(GZIP_ENV) \
	gzip -c >$(distdir)-bin-$(host_triplet).tar.gz; \
	$(am__remove_distdir)

data-dist:
	mkdir -p $(distdir); \
	for file in $(DATADISTFILES) ; do \
	  cp -pR $$file $(distdir)/ ; \
	done; \
	rm -rf `find $(distdir) -name CVS`; \
	$(AMTAR) chof - $(distdir) | GZIP=$(GZIP_ENV) \
	gzip -c >$(distdir)-data.tar.gz; \
	$(am__remove_distdir)

doc-dist:
	mkdir -p $(distdir); \
	for file in $(DOCDISTFILES) ; do \
	  cp -pR $$file $(distdir)/ ; \
	done; \
	rm -rf `find $(distdir) -name CVS`
	$(AMTAR) chof - $(distdir) | GZIP=$(GZIP_ENV) \
	gzip -c >$(distdir)-doc.tar.gz; \
	$(am__remove_distdir)

snapshot: distdir
	date_str=`date '+%Y%m%d-%H%M'`; \
	$(AMTAR) chof - $(distdir) | GZIP=$(GZIP_ENV) \
	gzip -c >$(distdir)-snapshot-$${date_str}.tar.gz; \
	$(am__remove_distdir)

bin-snapshot: all-am
	$(MAKE) $(AM_MAKE_FLAGS) prefix=$(prefix)/$(distdir) install-exec; \
	for file in $(BINDISTFILES) ; do \
	  cp -pR $$file $(distdir)/ ; \
	done; \
	rm -rf `find $(distdir) -name CVS`; \
	date_str=`date '+%Y%m%d-%H%M'`; \
	$(AMTAR) chof - $(distdir) | GZIP=$(GZIP_ENV) \
gzip -c >$(distdir)-bin-snapshot-$${date_str}.tar.gz; \
	$(am__remove_distdir)
