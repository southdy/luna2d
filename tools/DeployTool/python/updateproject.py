#-----------------------------------------------------------------------------
# luna2d DeployTool
# This is part of luna2d engine
# Copyright 2014-2016 Stepan Prokofjev
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#-----------------------------------------------------------------------------

import argparse
import shutil
import os
import utils
import constants
import update_wp


def main(args):
	if "LUNA2D_PATH" not in os.environ:
		print("LUNA2D_PATH environment value isn't set")
		exit(1)

	global ARGS
	global LUNA2D_PATH
	global CONFIG
	global PROJECT_CONFIG
	global CONSTANTS

	ARGS = args
	LUNA2D_PATH = os.path.abspath(os.environ["LUNA2D_PATH"])
	CONFIG = utils.load_json(ARGS.game_path + "/config.luna2d")
	#PROJECT_CONFIG = utils.load_json(ARGS.project_path + "/project-config.luna2d")
	CONSTANTS = {
		 "LUNA2D_PATH" : constants.PLATFORM[ARGS.platform]["LUNA2D_PATH"]
	}

	print("Updating libraries...")
	update_libs(args)

	print("Updating project..")
	if ARGS.platform == "wp":
		update_wp.do_update(ARGS, CONFIG)

	if args.update_assets:
		print("Updating assets..")
		update_assets(args)

	print("Done")

def update_assets(args):
	pass

def update_libs(args):
	pass

def parse_args():
	parser = argparse.ArgumentParser()
	parser.add_argument("--game_path", required=True)
	parser.add_argument("--project_path", required=True)
	parser.add_argument("--platform", required=True)
	parser.add_argument("--update_assets", default=True)

	return parser.parse_args()

main(parse_args())