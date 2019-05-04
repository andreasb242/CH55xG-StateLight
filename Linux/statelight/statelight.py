#!/usr/bin/python3

# State Light client
# License: GPL
# Author: Andreas Butti

import os
import signal
import gi
import json
import cairo
from collections import OrderedDict

gi.require_version('Gtk', '3.0')
gi.require_version('AppIndicator3', '0.1')

from gi.repository import Gtk as gtk
from gi.repository import Gdk as gdk
from gi.repository import AppIndicator3 as appindicator
from gi.repository import GObject

from gi.repository import GdkPixbuf

APPINDICATOR_ID = 'state-light'

lastIconId = 0

def main():
	global indicator
	global config

	with open('config.json') as json_data_file:
		config = json.load(json_data_file, object_pairs_hook=OrderedDict)

	indicator = appindicator.Indicator.new(APPINDICATOR_ID, os.path.abspath('statelight.svg'), appindicator.IndicatorCategory.SYSTEM_SERVICES)
	indicator.set_status(appindicator.IndicatorStatus.ACTIVE)
	indicator.set_menu(build_menu())
#	GObject.timeout_add(1000, updateTemperature)
	gtk.main()


def apply_color(item, color):
	global lastIconId
	print("-->" + str(color))
	pixbuf = draw_icon(color, True)
	
	lastIconId = lastIconId + 1
	if lastIconId > 2:
		lastIconId = 0
	
	iconpath = "/tmp/StateLightIcon" + str(lastIconId) + ".png"
	pixbuf.savev(iconpath, "png", [], [])
	indicator.set_icon_full(iconpath, str(color))


def draw_icon(color, small = False):
	surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 16, 16)
	cr = cairo.Context(surface)

	colr = ((color >> 16) & 0xff) / 255.0
	colg = ((color >>  8) & 0xff) / 255.0
	colb = ((color >>  0) & 0xff) / 255.0
	cr.set_source_rgb(colr, colg, colb)

	if small:
		cr.rectangle(4, 4, 8, 8)
		cr.fill()
	else:
		cr.rectangle(0, 0, 16, 16)
		cr.fill()

#	cr.move_to(0, 0)
#	cr.line_to(16, 16)
#	cr.stroke()

	return gdk.pixbuf_get_from_surface(surface, 0, 0, 16, 16)
	

def build_menu():
	menu = gtk.Menu()

	states = config['states']
	for s in states:
		color = int(states[s], 16)

		menu_state = gtk.ImageMenuItem(s)
		menu_state.connect('activate', apply_color, color)

		pixbuf = draw_icon(color)

		img = gtk.Image()
		img.set_from_pixbuf(pixbuf)
		menu_state.set_image(img)
		menu_state.set_always_show_image(True)
		menu.append(menu_state)


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








