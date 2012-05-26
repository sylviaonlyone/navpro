TEMPLATE = subdirs
win32:!contains(DISABLE,currera):exists(currera/lib): SUBDIRS += currera
