#-*- mode: makefile; -*-


PERL_MODULES = \
    lib/Bedrock/Apache/API.pm.in \
    lib/Bedrock/Apache/APIArgs.pm.in \
    lib/Bedrock/Apache/ExampleAPI.pm.in \
    lib/Bedrock/Apache/ExampleHandler.pm.in

GPERL_MODULES = $(PERL_MODULES:.pm.in=.pm)

@do_subst_command@

lib/Bedrock/Apache/API.pm: lib/Bedrock/Apache/APIArgs.pm

lib/Bedrock/Apache/ExampleAPI.pm: lib/Bedrock/Apache/API.pm

%.pm: %.pm.in
	test -d "$$(dirname "$@")" || $(INSTALL) -d "$$(dirname "$@")"
	$(do_subst) $< > $@
	perl -I lib -wc $@ || rm -f $@

VERSION = $(shell sh -c 'cat VERSION')

TARBALL = Bedrock-Apache-API-$(VERSION).tar.gz

TARBALL_DEPS = \
    buildspec.yml \
    $(GPERL_MODULES) \
    $(CONFIG_FILES) \
    requires \
    test-requires \
    README.md \
    VERSION

$(TARBALL): $(TARBALL_DEPS)
	make-cpan-dist.pl -b $<

README.md: lib/Bedrock/Apache/API.pm
	pod2markdown $< > $@
