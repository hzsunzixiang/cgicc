## $Id: Makefile.am,v 1.15 2014/04/23 20:55:11 sebdiaz Exp $

CLEANFILES = *~

EXTRA_DIST = styles.h

noinst_PROGRAMS = test.cgi dns.cgi upload.cgi cookie.cgi cardgame.cgi

VPATH = $(top_srcdir)/logger

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)

test_cgi_SOURCES = test.cpp
test_cgi_LDADD = ../cgicc/libcgicc.la @DEMO_LIBS@ $(top_srcdir)/logger/liblogger.a

dns_cgi_SOURCES = dns.cpp
dns_cgi_LDADD = ../cgicc/libcgicc.la @DEMO_LIBS@ @LIB_NET@ $(top_srcdir)/logger/liblogger.a

upload_cgi_SOURCES = upload.cpp
upload_cgi_LDADD = ../cgicc/libcgicc.la @DEMO_LIBS@ $(top_srcdir)/logger/liblogger.a

cookie_cgi_SOURCES = cookie.cpp
cookie_cgi_LDADD = ../cgicc/libcgicc.la @DEMO_LIBS@ $(top_srcdir)/logger/liblogger.a

cardgame_cgi_SOURCES = cardgame.cpp
cardgame_cgi_LDADD = ../cgicc/libcgicc.la @DEMO_LIBS@ $(top_srcdir)/logger/liblogger.a

