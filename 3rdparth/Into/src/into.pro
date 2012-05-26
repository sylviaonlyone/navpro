TEMPLATE = subdirs
SUBDIRS  = core \
           ydin \
           3rdparty \
           gui \
           script \
           plugins \
           declarative

exists(test/unit): SUBDIRS += test/unit
exists(tools): SUBDIRS += tools
exists(demos): SUBDIRS += demos
