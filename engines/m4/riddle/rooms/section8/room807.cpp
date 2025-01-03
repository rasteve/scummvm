/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/riddle/rooms/section8/room807.h"

#include "m4/adv_r/other.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {


// TODO : Refactor - This array is also present in walker.cpp
static const char *SAFARI_SHADOWS[5] = {
	"safari shadow 1", "safari shadow 2", "safari shadow 3",
	"safari shadow 4", "safari shadow 5"
};

void Room807::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room807::init() {
	if (inv_object_in_scene("wooden beam", 807)) {
		_807PostMach = series_show("807post", 4095, 0, -1, -1, 0, 100, 0, 0);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "wooden beam", false);
	}

	if (inv_object_in_scene("wooden post", 807)) {
		_807BeamMach = series_show("807beam", 4095, 0, -1, -1, 0, 100, 0, 0);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "wooden post", false);
	}

	if (_G(flags[V274])) {
		_807DoorMach = series_show("807door", 4095, 0, -1, -1, 0, 100, 0, 0);
		hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "corridor", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "chariot ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "north", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", false);

		if (player_been_here(807)) {
			_807DoorMach = series_show("807kart", 4095, 0, -1, -1, 100, 0, 0);
		}
	}

	_ripLowReachPos1Series = series_load("rip low reach pos1", -1, nullptr);
	_ripTrekHiReach2HndSeries = series_load("rip trek hi reach 2hnd", -1, nullptr);
	_ripTalkerPos5Series = series_load("RIP TALKER POS 5", -1, nullptr);
	_mctd82aSeries = series_load("mctd82a", -1, nullptr);
	_ripPos3LookAroundSeries = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
	_ripLooksAroundInAweSeries = series_load("RIP LOOKS AROUND IN AWE", -1, nullptr);

	series_play("807fire1", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("807fire2", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);

	_field34 = 0;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		digi_preload("950_s29", -1);

		if (_field38 != 0) {
			ws_demand_location(_G(my_walker), 476, 318);
			ws_demand_facing(_G(my_walker), 11);
			ws_hide_walker(_G(my_walker));
			_807Crnk2Mach = series_show("807rp05", 256, 0, -1, -1, 12, 100, 0, 0);
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadowMach = series_place_sprite(*SAFARI_SHADOWS, 0, 476, 318, _G(player_info).scale, 257);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx((kernel_trigger_create(6)));
			_G(kernel).trigger_mode = KT_PREPARSE;
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else if (inv_object_in_scene("crank", 807)) {
			_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		}

		if (_G(flags[V276]) != 0) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", false);
		} else {
			ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
			ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 560, 400, 11, Walker::player_walker_callback, "mc_trek");
		}

		break;

	case 808:
		player_set_commands_allowed(false);
		if (inv_object_in_scene("crank", 807)) {
			_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		}

		hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", false);
		_field38 = 0;
		ws_demand_location(_G(my_walker), 273, 270);
		ws_demand_facing(_G(my_walker), 5);

		if (_G(flags[V276]) != 0) {
			ws_walk(_G(my_walker), 250, 345, nullptr, 5, 2, true);
		} else {
			ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 295, 250, 5, Walker::player_walker_callback, "mc_trek");
			ws_walk(_mcTrekMach, 560, 400, nullptr, 5, 11, true);
			ws_walk(_G(my_walker), 250, 345, nullptr, -1, 2, true);
		}

		break;

	default:
		player_set_commands_allowed(false);
		ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
		ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
		if (inv_object_in_scene("crank", 807)) {
			_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		}

		hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", false);
		_field38 = 0;

		if (!player_been_here(807)) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 450, 60, 1, Walker::player_walker_callback, "mc_trek");
			ws_demand_location(_G(my_walker), 366, 345);
			ws_demand_facing(_G(my_walker), 11);
			ws_hide_walker(_G(my_walker));
			digi_preload("950_S33", -1);
			digi_preload("807_S01", -1);
			digi_play("950_S33", 2, 255, -1, -1);
			_807DoorMach = series_stream("807crush", 5, 0, 0);
			series_stream_break_on_frame(_807DoorMach, 60, 3);
		} else {
			ws_demand_location(_G(my_walker), 366, 500);
			ws_demand_facing(_G(my_walker), 1);

			if (_G(flags[V276]) != 0) {
				ws_walk(_G(my_walker), 366, 345, nullptr, 5, 2, true);
			} else {
				_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 450, 600, 1, Walker::player_walker_callback, "mc_trek");
				ws_walk(_G(my_walker), 366, 345, nullptr, -1, 2, true);
				ws_walk(_mcTrekMach, 560, 400, nullptr, 5, 11, true);
			}
		}

		break;
	}

	digi_play_loop("950_s29", 2, 127, -1, -1);
}

void Room807::pre_parser() {
	if (_G(flags[V274]) || inv_object_in_scene("wooden post", 807) || inv_object_in_scene("wooden beam", 807)) {
		if (player_said("gear", "stone block")) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		} else if (player_said("look at", "corridor")) {
			_G(player).walk_x = 285;
			_G(player).walk_y = 319;
		} else if (player_said("go", "south") || player_said("go", "north")) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	} else if (player_said("talk to", "mei chen") || player_said("wooden post", "crank")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	} else {
		intr_cancel_sentence();
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatchx(kernel_trigger_create(7));
		_G(kernel).trigger_mode = KT_PREPARSE;
	}
}

void Room807::parser() {
	_G(player).command_ready = false;

	if (_G(kernel).trigger == 747) {
		player_set_commands_allowed(true);
		_field34 = 0;
		return;
	}
	// TODO Not yet implemented
}

void Room807::daemon() {
	switch (_G(kernel.trigger)) {
	case 0:
		ws_unhide_walker(_G(my_walker));
		_807DoorMach = series_show("807door", 4095, 0, -1, -1, 0, 100, 0, 0);
		_G(flags[V274]) = 1;
		hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "corridor", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "chariot ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "north", false);
		ws_walk(_mcTrekMach, 560, 400, nullptr, 1, 11, true);

		break;

	case 1:
		digi_play("807m01", 1, 255, 2, -1);
		break;

	case 2:
		digi_play("807r01", 1, 255, 5, -1);
		break;

	case 3:
		digi_stop(2);
		digi_unload("950_s33");
		digi_play("807_s01", 2, 255, 4, -1);

		break;

	case 4:
		digi_unload("807_s01");
		break;

	case 5:
		player_set_commands_allowed(true);
		if (_G(flags[V276]) == 0) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", true);
			kernel_timing_trigger(imath_ranged_rand(1200, 1800), 13, nullptr);
		}

		break;

	case 6:
		kernel_timing_trigger(600, 7, "thunk!");
		break;

	case 7:
		if ((_G(flags[V274]) == 0) && !inv_object_in_scene("wooden beam", 807) && !inv_object_in_scene("wooden post", 807)) {
			if (_field34)
				kernel_timing_trigger(60, 7, "thunk!");
			else {
				player_set_commands_allowed(false);
				_G(flags[V274]) = 1;
				hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "corridor", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "chariot ", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "north", false);

				terminateMachine(_807Crnk2Mach);

				series_play("807rp04", 256, 2, 8, 5, 0, 100, 0, 0, 0, -1);

				terminateMachine(_807DoorMach);

				series_play("807close", 4095, 0, 10, 0, 0, 100, 0, 0, 0, -1);
				digi_play("807_s04", 2, 255, -1, -1);

				_field38 = 0;
			}
		}

		break;

	case 8:
		terminateMachine(_safariShadowMach);
		ws_unhide_walker(_G(my_walker));
		ws_demand_location(_G(my_walker), 476, 318);
		ws_demand_facing(_G(my_walker), 11);

		break;

	case 10:
		player_set_commands_allowed(true);
		digi_play("807_s04a", 2, 255, -1, -1);
		_807DoorMach = series_show("807door", 4095, 0, -1, -1, 0, 100, 0, 0);
		_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);

		break;

	case 11:
		_G(flags[V274]) = 1;
		disable_player_commands_and_fade_init(12);

		break;

	case 12:
		_field38 = 1;
		_G(flags[V274]) = 0;
		other_save_game_for_resurrection();
		_G(game).new_section = 4;
		_G(game).new_room = 413;

		break;

	case 13:
		if (player_commands_allowed() && checkStrings() && (_G(flags[V274]) != 0 || inv_object_in_scene("wooden post", 807) || inv_object_in_scene("wooden beam", 807))) {
			player_set_commands_allowed(false);
			intr_cancel_sentence();
			switch (imath_ranged_rand(1, 4)) {
			case 1:
				digi_play("950_s15", 2, 255, 14, -1);
				break;

			case 2:
				digi_play("950_s16", 2, 255, 14, -1);
				break;

			case 3:
				digi_play("950_s17", 2, 255, 14, -1);
				break;

			case 4:
			default:
				digi_play("950_s18", 2, 255, 14, -1);
				break;

			}

		} else {
			kernel_timing_trigger(60, 13, nullptr);
		}


		break;

	case 14:
		player_update_info(_G(my_walker), &_G(player_info));
		switch (_G(player_info).facing) {
		case 1:
		case 2:
		case 3:
		case 4:
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 15, 3, true);
			_807newFacing = 3;

			break;

		case 5:
			kernel_timing_trigger(30, 15, "phantom reaction");
			_807newFacing = 5;

			break;

		case 7:
			kernel_timing_trigger(30, 15, "phantom reaction");
			_807newFacing = 7;

			break;

		case 8:
		case 9:
		case 10:
		case 11:
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 15, 9, true);
			_807newFacing = 9;

			break;

		default:
			break;
		}


		break;

	case 15:
		_dword1A194C = 0;
		_dword1A1954 = imath_ranged_rand(1, 4);
		switch (_dword1A1954) {
		case 1:
			digi_play("COM052", 1, 255, 16, 997);
			break;

		case 2:
			digi_play("COM054", 1, 255, 16, 997);
			break;

		case 3:
			digi_play("COM056", 1, 255, 16, 997);
			break;

		case 4:
			digi_play("COM057", 1, 255, 16, 997);
			break;

		default:
			break;
		}

		setGlobals3(_mctd82aSeries, 1, 22);
		subD7916(_mcTrekMach, 18);
		switch (_807newFacing) {
		case 3:
		case 9:
			setGlobals3(_ripPos3LookAroundSeries, 1, 20);
			break;

		default:
			setGlobals3(_ripLooksAroundInAweSeries, 1, 14);
			break;
		}

		subD7916(_G(my_walker), 17);

		break;

	case 16:
		switch (_dword1A1954) {
		case 1:
			digi_play("COM053", 1, 255, -1, 997);
			break;

		case 2:
			digi_play("COM055", 1, 255, -1, 997);
			break;

		case 4:
			digi_play("COM058", 1, 255, -1, 997);
			break;

		default:
			break;
		}

		break;

	case 17:
		kernel_timing_trigger(imath_ranged_rand(90, 120), 19, nullptr);
		break;

	case 18:
		kernel_timing_trigger(imath_ranged_rand(90, 120), 20, nullptr);
		break;

	case 19:
		switch (_807newFacing) {
		case 3:
		case 9:
			setGlobals3(_ripPos3LookAroundSeries, 19, 1);
			break;

		default:
			setGlobals3(_ripLooksAroundInAweSeries, 13, 1);
			break;
		}

		subD7916(_G(my_walker), 21);

		break;

	case 20:
		setGlobals3(_mctd82aSeries, 22, 1);
		subD7916(_mcTrekMach, 21);

		break;

	case 21:
		++_dword1A194C;
		if (_dword1A194C == 2) {
			player_set_commands_allowed(true);
			ws_demand_facing(_G(my_walker), _807newFacing);
			kernel_timing_trigger(imath_ranged_rand(7200, 14400), 13, nullptr);
		}

		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
