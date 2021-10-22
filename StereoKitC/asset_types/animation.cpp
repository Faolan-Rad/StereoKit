#include "animation.h"
#include "model.h"
#include "mesh.h"
#include "../sk_math.h"
#include "../libraries/stref.h"

namespace sk {

array_t<model_t> animation_list = {};

///////////////////////////////////////////

int32_t anim_frame(const anim_curve_t *curve, int32_t prev_index, float t) {
	if (prev_index >= 0) {
		if (                                          curve->keyframe_times[prev_index  ] <= t && curve->keyframe_times[prev_index+1] > t) return prev_index;
		if (prev_index + 2 < curve->keyframe_count && curve->keyframe_times[prev_index+1] <= t && curve->keyframe_times[prev_index+2] > t) return prev_index + 1;
	}
	if (t <= curve->keyframe_times[0] || curve->keyframe_count == 1) return 0;
	if (t >= curve->keyframe_times[curve->keyframe_count - 1]) return curve->keyframe_count - 1;

	int32_t l = 0, r = curve->keyframe_count - 1;
	while (l <= r) {
		int32_t mid = (l+r) / 2;
		if      (curve->keyframe_times[mid] <= t && curve->keyframe_times[mid+1] > t) return mid;
		else if (curve->keyframe_times[mid] < t) l = mid + 1;
		else if (curve->keyframe_times[mid] > t) r = mid - 1;
		else log_err("Weird comparison");
	}
	log_err("uhh");
	return -1;
}

///////////////////////////////////////////

vec3 anim_curve_sample_f3(const anim_curve_t *curve, int32_t *prev_index, float t) {
	int32_t frame = anim_frame(curve, *prev_index, t);
	float   pct   = math_saturate((t - curve->keyframe_times[frame]) / (curve->keyframe_times[frame + 1] - curve->keyframe_times[frame]));
	vec3   *pts   = (vec3*)curve->keyframe_values;
	*prev_index = frame;

	switch (curve->interpolation) {
	case anim_interpolation_linear: return vec3_lerp(pts[frame], pts[frame + 1], pct); break;
	case anim_interpolation_step:   return pts[frame]; break;
	case anim_interpolation_cubic:  return vec3_lerp(pts[frame], pts[frame + 1], pct); break;
	default: return pts[frame]; break;
	}
}

///////////////////////////////////////////

quat anim_curve_sample_f4(const anim_curve_t *curve, int32_t *prev_index, float t) {
	int32_t frame = anim_frame(curve, *prev_index, t);
	float   pct   = math_saturate((t - curve->keyframe_times[frame]) / (curve->keyframe_times[frame + 1] - curve->keyframe_times[frame]));
	quat   *pts   = (quat*)curve->keyframe_values;
	*prev_index = frame;

	switch (curve->interpolation) {
	case anim_interpolation_linear: return quat_slerp(pts[frame], pts[frame + 1], pct); break;
	case anim_interpolation_step:   return pts[frame]; break;
	case anim_interpolation_cubic:  return quat_slerp(pts[frame], pts[frame + 1], pct); break;
	default: return pts[frame]; break;
	}
}

///////////////////////////////////////////

static void anim_update_transforms(model_t model, model_node_id node_id, bool dirty_world) {
	while (node_id != -1) {
		model_node_t     *node  = &model->nodes[node_id];
		anim_transform_t *tr    = &model->anim_inst.node_transforms[node_id];
		bool              dirty = dirty_world;

		// Only update this node_id if the animation touched the transform
		if (tr->dirty) {
			node->transform_local = matrix_trs(tr->translation, tr->rotation, tr->scale);
			tr->dirty             = false;
			dirty                 = true;
		}

		// If this node_id or a parent node_id was touched, we need to update the world
		// transform.
		if (dirty == true) {
			if (node->parent >= 0) node->transform_model = node->transform_local * model->nodes[node->parent].transform_model;
			else                   node->transform_model = node->transform_local;

			if (node->visual >= 0)
				model->visuals[node->visual].transform_model = node->transform_model;
		}
		anim_update_transforms(model, node->child, dirty);
		node_id = node->sibling;
	}
}

///////////////////////////////////////////

void anim_update_model(model_t model) {
	if (model->anim_inst.anim_id < 0) return;

	// Don't update more than once per frame
	float curr_time = model->anim_inst.mode == anim_mode_manual 
		? model->anim_inst.start_time
		: time_getf();
	if (model->anim_inst.last_update == curr_time) return;
	model->anim_inst.last_update = curr_time;
	model->transforms_changed    = true;

	anim_t *anim = &model->anim_data.anims[model->anim_inst.anim_id];
	float   time = model_anim_active_time(model);

	for (size_t i = 0; i < anim->curves.count; i++) {
		model_node_id node = anim->curves[i].node_id;

		model->anim_inst.node_transforms[node].dirty = true;
		switch (anim->curves[i].applies_to) {
		case anim_element_translation: model->anim_inst.node_transforms[node].translation = anim_curve_sample_f3(&anim->curves[i], &model->anim_inst.curve_last_keyframe[i], time); break;
		case anim_element_scale:       model->anim_inst.node_transforms[node].scale       = anim_curve_sample_f3(&anim->curves[i], &model->anim_inst.curve_last_keyframe[i], time); break;
		case anim_element_rotation:    model->anim_inst.node_transforms[node].rotation    = anim_curve_sample_f4(&anim->curves[i], &model->anim_inst.curve_last_keyframe[i], time); break;
		}
	}
	anim_update_transforms(model, model_node_get_root(model), false);
}

///////////////////////////////////////////

void anim_update_skin(model_t model) {
	animation_list.add(model);
}

///////////////////////////////////////////

void _anim_update_skin(model_t &model) {
	if (model->anim_inst.anim_id < 0) return;

	for (size_t i = 0; i < model->anim_inst.skinned_mesh_count; i++) { 
		matrix root = matrix_invert(model_node_get_transform_model(model, model->anim_data.skeletons[i].skin_node));
		for (size_t b = 0; b < model->anim_data.skeletons[i].bone_count; b++) {
			model->anim_inst.skinned_meshes[i].bone_transforms[b] = model_node_get_transform_model(model, model->anim_data.skeletons[i].bone_to_node_map[b]) * root;
		}
		mesh_update_skin(
			model->anim_inst.skinned_meshes[i].modified_mesh,
			model->anim_inst.skinned_meshes[i].bone_transforms,
			model->anim_data.skeletons     [i].bone_count);
	}
}

///////////////////////////////////////////

void anim_inst_play(model_t model, int32_t anim_id, anim_mode_ mode) {
	if (anim_id < 0 || anim_id >= model->anim_data.anims.count) {
		log_err("Attempted to play an invalid animation id.");
		return;
	}
	if (model->anim_inst.curve_last_keyframe == nullptr) {
		model->anim_inst.curve_last_keyframe = sk_malloc_t(int32_t, model->anim_data.anims[anim_id].curves.count);
		memset(model->anim_inst.curve_last_keyframe, 0, sizeof(int32_t) * model->anim_data.anims[anim_id].curves.count);
	}
	if (model->anim_inst.node_transforms == nullptr) {
		model->anim_inst.node_transforms = sk_malloc_t(anim_transform_t, model->nodes.count);
		for (size_t i = 0; i < model->nodes.count; i++) {
			matrix_decompose(model->nodes[i].transform_local,
				model->anim_inst.node_transforms[i].translation,
				model->anim_inst.node_transforms[i].scale,
				model->anim_inst.node_transforms[i].rotation);
		}
	}
	if (model->anim_inst.skinned_meshes == nullptr) {
		model->anim_inst.skinned_mesh_count = model->anim_data.skeletons.count;
		model->anim_inst.skinned_meshes     = sk_malloc_t(anim_inst_subset_t, model->anim_inst.skinned_mesh_count);
		for (int32_t i = 0; i < model->anim_inst.skinned_mesh_count; i++) {
			model->anim_inst.skinned_meshes[i].original_mesh   = model_node_get_mesh(model, model->anim_data.skeletons[i].skin_node);
			model->anim_inst.skinned_meshes[i].modified_mesh   = mesh_copy(model->anim_inst.skinned_meshes[i].original_mesh);
			model->anim_inst.skinned_meshes[i].bone_transforms = sk_malloc_t(matrix, model->anim_data.skeletons[i].bone_count);
			model_node_set_mesh(model, model->anim_data.skeletons[i].skin_node, model->anim_inst.skinned_meshes[i].modified_mesh);
		}
	}
	model->anim_inst.start_time  = time_getf();
	model->anim_inst.last_update = model->anim_inst.start_time;
	model->anim_inst.anim_id     = anim_id;
	model->anim_inst.mode        = mode;
}

///////////////////////////////////////////

void anim_inst_destroy(anim_inst_t *inst) {
	for (int32_t i = 0; i < inst->skinned_mesh_count; i++) {
		free(inst->skinned_meshes[i].bone_transforms);
		mesh_release(inst->skinned_meshes[i].original_mesh);
	}
	free(inst->skinned_meshes);
	free(inst->curve_last_keyframe);
	free(inst->node_transforms);

	*inst = {};
	inst->anim_id = -1;
}

///////////////////////////////////////////

void anim_data_destroy(anim_data_t *data) {
	for (size_t i = 0; i < data->anims.count; i++) {
		for (size_t c = 0; c < data->anims[i].curves.count; c++) {
			free(data->anims[i].curves[c].keyframe_values);
			free(data->anims[i].curves[c].keyframe_times);
		}
		data->anims[i].curves.free();
	}
	for (size_t i = 0; i < data->skeletons.count; i++) {
		free(data->skeletons[i].bone_to_node_map);
	}
	data->anims    .free();
	data->skeletons.free();
}

///////////////////////////////////////////

anim_data_t anim_data_copy(anim_data_t *data) {
	anim_data_t result = {};
	
	if (data->anims.count > 0) {
		result.anims.resize(data->anims.count);
		for (size_t i = 0; i < data->anims.count; i++) {
			anim_t &anim_src = data->anims[i];
			anim_t  anim = {};
			anim.duration = anim_src.duration;
			anim.name     = string_copy(anim_src.name);
			anim.curves.resize(anim_src.curves.count);
			for (size_t c = 0; c < anim_src.curves.count; c++) {
				anim_curve_t &curve_src = anim_src.curves[c];
				anim_curve_t  curve = {};
				curve.applies_to     = curve_src.applies_to;
				curve.interpolation  = curve_src.interpolation;
				curve.keyframe_count = curve_src.keyframe_count;
				curve.node_id        = curve_src.node_id;

				curve.keyframe_times = sk_malloc_t(float, curve.keyframe_count);
				memcpy(curve.keyframe_times, curve_src.keyframe_times, sizeof(float) * curve.keyframe_count);

				size_t value_size = 0;
				switch (curve.applies_to) {
				case anim_element_rotation:    value_size = sizeof(quat) * curve.keyframe_count; break;
				case anim_element_scale:       value_size = sizeof(vec3) * curve.keyframe_count; break;
				case anim_element_translation: value_size = sizeof(vec3) * curve.keyframe_count; break;
				default: log_errf("anim_data_copy doesn't implement anim_element_weights yet!");
				}
				curve.keyframe_values = sk_malloc(value_size);
				memcpy(curve.keyframe_values, curve_src.keyframe_values, value_size);

				anim.curves.add(curve);
			}
			result.anims.add(anim);
		}
	}

	if (data->skeletons.count > 0) {
		result.skeletons.resize(data->skeletons.count);
		for (size_t i = 0; i < data->skeletons.count; i++) {
			anim_skeleton_t &skel_src = data->skeletons[i];
			anim_skeleton_t  skel     = {};
			skel.bone_count       = skel_src.bone_count;
			skel.skin_node        = skel_src.skin_node;
			skel.bone_to_node_map = sk_malloc_t(int32_t, skel.bone_count);
			memcpy(skel.bone_to_node_map, skel_src.bone_to_node_map, sizeof(int32_t) * skel.bone_count);

			result.skeletons.add(skel);
		}
	}

	return result;
}

///////////////////////////////////////////

void anim_update() {
	animation_list.each(_anim_update_skin);
	animation_list.clear();
}

///////////////////////////////////////////

void anim_shutdown() {
	animation_list.free();
}

} // namespace sk
