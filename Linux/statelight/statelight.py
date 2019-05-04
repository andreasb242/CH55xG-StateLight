#!/usr/bin/python3

# State Light client
# License: GPL
# Author: Andreas Butti

import os
import signal
import gi
import json
import cairo

gi.require_version('Gtk', '3.0')
gi.require_version('AppIndicator3', '0.1')

from gi.repository import Gtk as gtk
from gi.repository import Gdk as gdk
from gi.repository import AppIndicator3 as appindicator
from gi.repository import GObject

from gi.repository import GdkPixbuf

APPINDICATOR_ID = 'state-light'

def main():
	global indicator
	global config

	with open('config.json') as json_data_file:
		config = json.load(json_data_file)

	indicator = appindicator.Indicator.new(APPINDICATOR_ID, os.path.abspath('statelight.svg'), appindicator.IndicatorCategory.SYSTEM_SERVICES)
	indicator.set_status(appindicator.IndicatorStatus.ACTIVE)
	indicator.set_menu(build_menu())
#	GObject.timeout_add(1000, updateTemperature)
	gtk.main()

def build_menu():
	menu = gtk.Menu()

	states = config['states']
	for s in states:
		menu_state = gtk.ImageMenuItem(s)
#		item_quit.connect('activate', quit)
		
#		pixbuf = GdkPixbuf.Pixbuf.new_from_file('statelight.svg')



		surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 16, 16)
		cr = cairo.Context(surface)
		cr.set_source_rgb(0.9 , 0 , 0)
		cr.move_to(0, 0)
		cr.line_to(16, 16)
		cr.stroke()

		pixbuf = gdk.pixbuf_get_from_surface(surface, 0, 0, 16, 16)

		img = gtk.Image()
		img.set_from_pixbuf(pixbuf)
		menu_state.set_image(img)
		menu_state.set_always_show_image(True)
		menu.append(menu_state)


#		print(s + "=>" + states[s])


	menu.append(gtk.SeparatorMenuItem())

	item_quit = gtk.MenuItem('Quit')
	item_quit.connect('activate', quit)
	menu.append(item_quit)


	menu.show_all()
	return menu

def quit(_):
	gtk.main_quit()

if __name__ == "__main__":
	signal.signal(signal.SIGINT, signal.SIG_DFL)
	main()








