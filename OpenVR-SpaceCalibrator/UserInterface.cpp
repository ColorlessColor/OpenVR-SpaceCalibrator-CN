#include "stdafx.h"
#include "UserInterface.h"
#include "Calibration.h"
#include "Configuration.h"
#include "VRState.h"
#include "CalibrationMetrics.h"
#include "../Version.h"

#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <imgui/imgui.h>
#include "imgui_extensions.h"

void TextWithWidth(const char *label, const char *text, float width);

VRState LoadVRState();
void BuildSystemSelection(const VRState &state);
void BuildDeviceSelections(const VRState &state);
void BuildProfileEditor();
void BuildMenu(bool runningInOverlay);

static const ImGuiWindowFlags bareWindowFlags =
	ImGuiWindowFlags_NoTitleBar |
	ImGuiWindowFlags_NoResize |
	ImGuiWindowFlags_NoMove |
	ImGuiWindowFlags_NoScrollbar |
	ImGuiWindowFlags_NoScrollWithMouse |
	ImGuiWindowFlags_NoCollapse;

void BuildContinuousCalDisplay();
void ShowVersionLine();

static bool runningInOverlay;

void BuildMainWindow(bool runningInOverlay_)
{
	runningInOverlay = runningInOverlay_;
	bool continuousCalibration = CalCtx.state == CalibrationState::Continuous || CalCtx.state == CalibrationState::ContinuousStandby;
	
	auto &io = ImGui::GetIO();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

	if (!ImGui::Begin("OpenVRSpaceCalibrator", nullptr, bareWindowFlags))
	{
		ImGui::End();
		return;
	}

	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::GetStyleColorVec4(ImGuiCol_Button));

	if (continuousCalibration) {
		BuildContinuousCalDisplay();
	}
	else {
		auto state = LoadVRState();

		ImGui::BeginDisabled(CalCtx.state == CalibrationState::Continuous);
		BuildSystemSelection(state);
		BuildDeviceSelections(state);
		ImGui::EndDisabled();
		BuildMenu(runningInOverlay);
	}

	ShowVersionLine();

	ImGui::PopStyleColor();
	ImGui::End();
}

void ShowVersionLine() {
	ImGui::SetNextWindowPos(ImVec2(10.0f, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing()));
	if (!ImGui::BeginChild("bottom line", ImVec2(ImGui::GetWindowWidth() - 20.0f, ImGui::GetFrameHeightWithSpacing() * 2), false)) {
		ImGui::EndChild();
		return;
	}
	ImGui::Text("OpenVR Space Calibrator CN v" SPACECAL_VERSION_STRING);
	if (runningInOverlay)
	{
		ImGui::SameLine();
		ImGui::Text(u8"- �ر� VR ���Ӳ���ͨ�����ٿ�");
	}
	ImGui::EndChild();
}

void CCal_BasicInfo();
void CCal_DrawSettings();

void BuildContinuousCalDisplay() {
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetWindowSize());
	ImGui::SetNextWindowBgAlpha(1);
	if (!ImGui::Begin(u8"����У׼", nullptr,
		bareWindowFlags & ~ImGuiWindowFlags_NoTitleBar
	)) {
		ImGui::End();
		return;
	}

	ImVec2 contentRegion;
	contentRegion.x = ImGui::GetWindowContentRegionWidth();
	contentRegion.y = ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * 2.1;

	if (!ImGui::BeginChild("CCalDisplayFrame", contentRegion, false)) {
		ImGui::EndChild();
		return;
	}

	if (ImGui::BeginTabBar("CCalTabs", 0)) {
		if (ImGui::BeginTabItem(u8"״̬")) {
			CCal_BasicInfo();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(u8"��ϸ��Ϣ")) {
			ShowCalibrationDebug(2, 3);
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem(u8"����")) {
			CCal_DrawSettings();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::EndChild();

	ShowVersionLine();

	ImGui::End();
}

static void ScaledDragFloat(const char* label, double& f, double scale, double min, double max, int flags = ImGuiSliderFlags_AlwaysClamp) {
	float v = (float) (f * scale);
	std::string labelStr = std::string(label);

	// If starts with ##, just do a normal SliderFloat
	if (labelStr.size() > 2 && labelStr[0] == '#' && labelStr[1] == '#') {
		ImGui::SliderFloat(label, &v, (float)min, (float)max, "%1.2f", flags);
	} else {
		// Otherwise do funny
		ImGui::Text(label);
		ImGui::SameLine();
		ImGui::PushID((std::string(label) + "_id").c_str());
		// Line up to a column, multiples of 100
		constexpr uint32_t LABEL_CURSOR = 100;
		uint32_t cursorPosX = ImGui::GetCursorPosX();
		uint32_t roundedPosition = ((cursorPosX + LABEL_CURSOR / 2) / LABEL_CURSOR) * LABEL_CURSOR;
		ImGui::SetCursorPosX(roundedPosition);
		ImGui::SliderFloat((std::string("##") + label).c_str(), &v, (float)min, (float)max, "%1.2f", flags);
		ImGui::PopID();
	}
	
	f = v / scale;
}

void DrawVectorElement(std::string id, const char* text, double* value);

void CCal_DrawSettings() {

	// panel size for boxes
	ImVec2 panel_size { ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, 0 };

	ImGui::BeginGroupPanel(u8"��ʾ", panel_size);
	ImGui::Text(u8"�������ͣ�����������˽������Ϣ��");
	ImGui::EndGroupPanel();


	// @TODO: Group in UI

	// Section: Alignment speeds
	{
		ImGui::BeginGroupPanel(u8"У׼�ٶ�", panel_size);

		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		ImGui::TextWrapped(
			u8"�ռ�У׼�� �ڷ���Ư��ʱʹ��������ֲ�ͬ���ٶȽ�У׼�ϻص���ȷλ�á���Щ���ÿ���У׼ƫ���ԶʱӦѡ������У׼�ٶȣ�������Decel�����Ǹ��죨Fast����"
			//"����"
		);
		ImGui::PopStyleColor();

		if (ImGui::BeginTable("SpeedThresholds", 3, 0)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(u8"λ�� (mm)");
			ImGui::TableSetColumnIndex(2);
			ImGui::Text(u8"��ת (degrees)");


			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(u8"����");
			ImGui::TableSetColumnIndex(1);
			ScaledDragFloat("##TransDecel", CalCtx.alignmentSpeedParams.thr_trans_tiny, 1000.0, 0, 20.0);
			ImGui::TableSetColumnIndex(2);
			ScaledDragFloat("##RotDecel", CalCtx.alignmentSpeedParams.thr_rot_tiny, 180.0 / EIGEN_PI, 0, 5.0);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(u8"����");
			ImGui::TableSetColumnIndex(1);
			ScaledDragFloat("##TransSlow", CalCtx.alignmentSpeedParams.thr_trans_small, 1000.0,
				CalCtx.alignmentSpeedParams.thr_trans_tiny * 1000.0, 20.0);
			ImGui::TableSetColumnIndex(2);
			ScaledDragFloat("##RotSlow", CalCtx.alignmentSpeedParams.thr_rot_small, 180.0 / EIGEN_PI,
				CalCtx.alignmentSpeedParams.thr_rot_tiny * (180.0 / EIGEN_PI), 10.0);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(u8"����");
			ImGui::TableSetColumnIndex(1);
			ScaledDragFloat("##TransFast", CalCtx.alignmentSpeedParams.thr_trans_large, 1000.0,
				CalCtx.alignmentSpeedParams.thr_trans_small * 1000.0, 50.0);
			ImGui::TableSetColumnIndex(2);
			ScaledDragFloat("##RotFast", CalCtx.alignmentSpeedParams.thr_rot_large, 180.0 / EIGEN_PI,
				CalCtx.alignmentSpeedParams.thr_rot_small * (180.0 / EIGEN_PI), 20.0);

			ImGui::EndTable();
		}

		ImGui::EndGroupPanel();
	}

	// Section: Alignment speeds
	{
		ImGui::BeginGroupPanel(u8"�����ٶ�", panel_size);

		// ImGui::Separator();
		// ImGui::Text("Alignment speeds");
		ScaledDragFloat(u8"����", CalCtx.alignmentSpeedParams.align_speed_tiny, 1.0, 0, 2.0, 0);
		ScaledDragFloat(u8"����", CalCtx.alignmentSpeedParams.align_speed_small, 1.0, 0, 2.0, 0);
		ScaledDragFloat(u8"����", CalCtx.alignmentSpeedParams.align_speed_large, 1.0, 0, 2.0, 0);
		
		ImGui::EndGroupPanel();
	}
	

	// Section: Continuous Calibration settings
	{
		ImGui::BeginGroupPanel(u8"����У׼", panel_size);

		{
			// @TODO: Reduce code duplication (tooltips)
			// Recalibration threshold
			ImGui::Text(u8"����У׼��ֵ");
			ImGui::SameLine();
			ImGui::PushID("recalibration_threshold");
			ImGui::SliderFloat("##recalibration_threshold_slider", &CalCtx.continuousCalibrationThreshold, 1.01f, 10.0f, "%1.1f", 0);
			if (ImGui::IsItemHovered(0)) {
				ImGui::SetTooltip(u8"���ڵ��������¶��������֮ǰ��У׼����ﵽ��׼ȷ�ȡ�\n"
					u8"���ߵ�ֵ�ή��У׼������Ƶ�ʣ�����ܶԾ��нϴ����Ư�Ƶ�ϵͳ�а�����");
			}
			ImGui::PopID();

			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			ImGui::TextWrapped(u8"���� �ռ�У׼�� ͬ����Ϸ�ռ��Ƶ�ʡ�");
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered(0)) {
				ImGui::SetTooltip(u8"���ڵ��������¶��������֮ǰ��У׼����ﵽ��׼ȷ�ȡ�\n"
					u8"���ߵ�ֵ�ή��У׼������Ƶ�ʣ�����ܶԾ��нϴ����Ư�Ƶ�ϵͳ�а�����");
			}
		}

		{
			// Tracker offset
			// ImVec2 panel_size_inner { ImGui::GetCurrentWindow()->DC.ItemWidth, 0};
			ImVec2 panel_size_inner { panel_size.x - 11 * 2, 0};
			ImGui::BeginGroupPanel(u8"׷����ƫ��", panel_size_inner);
			DrawVectorElement("cc_tracker_offset", "X", &CalCtx.continuousCalibrationOffset.x());
			DrawVectorElement("cc_tracker_offset", "Y", &CalCtx.continuousCalibrationOffset.y());
			DrawVectorElement("cc_tracker_offset", "Z", &CalCtx.continuousCalibrationOffset.z());
			ImGui::EndGroupPanel();
		}

		{
			// Playspace offset
			ImVec2 panel_size_inner{ panel_size.x - 11 * 2, 0 };
			ImGui::BeginGroupPanel(u8"����ռ�����", panel_size_inner);
			DrawVectorElement("cc_playspace_scale", u8"����ռ�����", &CalCtx.calibratedScale);
			ImGui::EndGroupPanel();
		}

		ImGui::EndGroupPanel();
	}

	ImGui::NewLine();
	ImGui::Indent();
	if (ImGui::Button(u8"��������")) {
		CalCtx.ResetConfig();
	}
	ImGui::Unindent();
	ImGui::NewLine();

	// Section: Contributors credits
	{
		ImGui::BeginGroupPanel("����", panel_size);

		ImGui::TextDisabled("tach");
		ImGui::TextDisabled("pushrax");
		ImGui::TextDisabled("bd_");
		ImGui::TextDisabled("ArcticFox");
		ImGui::TextDisabled("hekky");
		ImGui::TextDisabled("pimaker");

		ImGui::EndGroupPanel();
	}
}

void DrawVectorElement(std::string id, const char* text, double* value) {
	constexpr float CONTINUOUS_CALIBRATION_TRACKER_OFFSET_DELTA = 0.01f;

	ImGui::Text(text);

	ImGui::SameLine();

	ImGui::PushID((id + text + "_btn_reset").c_str());
	if (ImGui::Button(" 0 ")) {
		*value *= 0;
	}
	ImGui::PopID();
	ImGui::SameLine();
	if (ImGui::ArrowButton((id + text + "_decrease").c_str(), ImGuiDir_Down)) {
		*value -= CONTINUOUS_CALIBRATION_TRACKER_OFFSET_DELTA;
	}
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::PushID((id + text + "_text_field").c_str());
	ImGui::InputDouble("##label", value, 0, 0, "%.2f");
	ImGui::PopID();
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::ArrowButton((id + text + "_increase").c_str(), ImGuiDir_Up)) {
		*value += CONTINUOUS_CALIBRATION_TRACKER_OFFSET_DELTA;
	}
}

void CCal_BasicInfo() {
	if (ImGui::BeginTable("DeviceInfo", 2, 0)) {
		ImGui::TableSetupColumn(u8"���豸");
		ImGui::TableSetupColumn(u8"Ŀ���豸");
		ImGui::TableHeadersRow();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::BeginGroup();
		ImGui::Text("%s / %s / %s",
			CalCtx.referenceStandby.trackingSystem.c_str(),
			CalCtx.referenceStandby.model.c_str(),
			CalCtx.referenceStandby.serial.c_str()
		);
		const char* status;
		if (CalCtx.referenceID < 0) {
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF000080);
			status = u8"δ�ҵ�";
		}
		else if (!CalCtx.ReferencePoseIsValid()) {
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFFFF0080);
			status = u8"δ��׷��";
		}
		else {
			status = u8"׼������";
		}
		ImGui::Text(u8"״̬�� %s", status);
		ImGui::EndGroup();

		ImGui::TableSetColumnIndex(1);
		ImGui::BeginGroup();
		ImGui::Text("%s / %s / %s",
			CalCtx.targetStandby.trackingSystem.c_str(),
			CalCtx.targetStandby.model.c_str(),
			CalCtx.targetStandby.serial.c_str()
		);
		if (CalCtx.targetID < 0) {
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF000080);
			status = u8"δ�ҵ�";
		}
		else if (!CalCtx.TargetPoseIsValid()) {
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFFFF0080);
			status = u8"δ��׷��";
		}
		else {
			status = u8"׼������";
		}
		ImGui::Text(u8"״̬�� %s", status);
		ImGui::EndGroup();

		ImGui::EndTable();
	}

	float width = ImGui::GetWindowContentRegionWidth(), scale = 1.0f;

	if (ImGui::BeginTable("##CCal_Cancel", Metrics::enableLogs ? 3 : 2, 0, ImVec2(width * scale, ImGui::GetTextLineHeight() * 2))) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if (ImGui::Button(u8"ȡ������У׼", ImVec2(-FLT_MIN, 0.0f))) {
			EndContinuousCalibration();
		}

		ImGui::TableSetColumnIndex(1);
		if (ImGui::Button(u8"���ԣ�ǿ���ж�У׼", ImVec2(-FLT_MIN, 0.0f))) {
			DebugApplyRandomOffset();
		}

		if (Metrics::enableLogs) {
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button(u8"���ԣ������־", ImVec2(-FLT_MIN, 0.0f))) {
				Metrics::WriteLogAnnotation("MARK LOGS");
			}
		}

		ImGui::EndTable();
	}

	ImGui::Checkbox(u8"����׷����", &CalCtx.quashTargetInContinuous);
	ImGui::SameLine();
	ImGui::Checkbox(u8"��̬����У׼", &CalCtx.enableStaticRecalibration);
	ImGui::SameLine();
	ImGui::Checkbox(u8"����������־", &Metrics::enableLogs);
	ImGui::SameLine();
	ImGui::Checkbox(u8"�������λ��", &CalCtx.lockRelativePosition);
	ImGui::SameLine();
	ImGui::Checkbox(u8"��Ҫ���", &CalCtx.requireTriggerPressToApply);

	// Status field...

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 1));

	for (const auto& msg : CalCtx.messages) {
		if (msg.type == CalibrationContext::Message::String) {
			ImGui::TextWrapped("> %s", msg.str.c_str());
		}
	}

	ImGui::PopStyleColor();

	ShowCalibrationDebug(1, 3);
}

void BuildMenu(bool runningInOverlay)
{
	auto &io = ImGui::GetIO();
	ImGuiStyle &style = ImGui::GetStyle();
	ImGui::Text("");

	if (CalCtx.state == CalibrationState::None)
	{
		if (CalCtx.validProfile && !CalCtx.enabled)
		{
			ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1), u8"����ͷ���豸 (%s) δ�ҵ��������ļ��ѽ���", CalCtx.referenceTrackingSystem.c_str());
			ImGui::Text("");
		}

		float width = ImGui::GetWindowContentRegionWidth(), scale = 1.0f;
		if (CalCtx.validProfile)
		{
			width -= style.FramePadding.x * 4.0f;
			scale = 1.0f / 4.0f;
		}

		if (ImGui::Button(u8"��ʼУ׼", ImVec2(width * scale, ImGui::GetTextLineHeight() * 2)))
		{
			ImGui::OpenPopup("Calibration Progress");
			StartCalibration();
		}

		ImGui::SameLine();
		if (ImGui::Button(u8"����У׼", ImVec2(width * scale, ImGui::GetTextLineHeight() * 2))) {
			StartContinuousCalibration();
		}

		if (CalCtx.validProfile)
		{
			ImGui::SameLine();
			if (ImGui::Button(u8"�༭У׼", ImVec2(width * scale, ImGui::GetTextLineHeight() * 2)))
			{
				CalCtx.state = CalibrationState::Editing;
			}

			ImGui::SameLine();
			if (ImGui::Button(u8"���У׼", ImVec2(width * scale, ImGui::GetTextLineHeight() * 2)))
			{
				CalCtx.Clear();
				SaveProfile(CalCtx);
			}
		}

		width = ImGui::GetWindowContentRegionWidth();
		scale = 1.0f;
		if (CalCtx.chaperone.valid)
		{
			width -= style.FramePadding.x * 2.0f;
			scale = 0.5;
		}

		ImGui::Text("");
		if (ImGui::Button(u8"�� Chaperone �߽縴�Ƶ������ļ�", ImVec2(width * scale, ImGui::GetTextLineHeight() * 2)))
		{
			LoadChaperoneBounds();
			SaveProfile(CalCtx);
		}

		if (CalCtx.chaperone.valid)
		{
			ImGui::SameLine();
			if (ImGui::Button(u8"ճ�� Chaperone �߽�", ImVec2(width * scale, ImGui::GetTextLineHeight() * 2)))
			{
				ApplyChaperoneBounds();
			}

			if (ImGui::Checkbox(u8" �ڼ���������ʱ�Զ�ճ�� Chaperone �߽�", &CalCtx.chaperone.autoApply))
			{
				SaveProfile(CalCtx);
			}
		}

		ImGui::Text("");
		auto speed = CalCtx.calibrationSpeed;

		ImGui::Columns(4, NULL, false);
		ImGui::Text(u8"У׼�ٶ�");

		ImGui::NextColumn();
		if (ImGui::RadioButton(u8" ����          ", speed == CalibrationContext::FAST))
			CalCtx.calibrationSpeed = CalibrationContext::FAST;

		ImGui::NextColumn();
		if (ImGui::RadioButton(u8" ����          ", speed == CalibrationContext::SLOW))
			CalCtx.calibrationSpeed = CalibrationContext::SLOW;

		ImGui::NextColumn();
		if (ImGui::RadioButton(u8" ����     ", speed == CalibrationContext::VERY_SLOW))
			CalCtx.calibrationSpeed = CalibrationContext::VERY_SLOW;

		ImGui::Columns(1);
	}
	else if (CalCtx.state == CalibrationState::Editing)
	{
		BuildProfileEditor();

		if (ImGui::Button(u8"���������ļ�", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeight() * 2)))
		{
			SaveProfile(CalCtx);
			CalCtx.state = CalibrationState::None;
		}
	}
	else
	{
		ImGui::Button("У׼������...", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeight() * 2));
	}

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 40.0f, io.DisplaySize.y - 40.0f), ImGuiCond_Always);
	if (ImGui::BeginPopupModal("Calibration Progress", nullptr, bareWindowFlags))
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImVec4(0, 0, 0, 1));
		for (auto &message : CalCtx.messages)
		{
			switch (message.type)
			{
			case CalibrationContext::Message::String:
				ImGui::TextWrapped(message.str.c_str());
				break;
			case CalibrationContext::Message::Progress:
				float fraction = (float)message.progress / (float)message.target;
				ImGui::Text("");
				ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), "");
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetFontSize() - style.FramePadding.y * 2);
				ImGui::Text(" %d%%", (int)(fraction * 100));
				break;
			}
		}
		ImGui::PopStyleColor();

		if (CalCtx.state == CalibrationState::None)
		{
			ImGui::Text("");
			if (ImGui::Button("Close", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeight() * 2)))
				ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void BuildSystemSelection(const VRState &state)
{
	if (state.trackingSystems.empty())
	{
		ImGui::Text("No tracked devices are present");
		return;
	}

	ImGuiStyle &style = ImGui::GetStyle();
	float paneWidth = ImGui::GetWindowContentRegionWidth() / 2 - style.FramePadding.x;

	TextWithWidth("ReferenceSystemLabel", u8"���豸", paneWidth);
	ImGui::SameLine();
	TextWithWidth("TargetSystemLabel", u8"Ŀ���豸", paneWidth);

	int currentReferenceSystem = -1;
	int currentTargetSystem = -1;
	int firstReferenceSystemNotTargetSystem = -1;

	std::vector<const char *> referenceSystems;
	for (auto &str : state.trackingSystems)
	{
		if (str == CalCtx.referenceTrackingSystem)
		{
			currentReferenceSystem = (int) referenceSystems.size();
		}
		else if (firstReferenceSystemNotTargetSystem == -1 && str != CalCtx.targetTrackingSystem)
		{
			firstReferenceSystemNotTargetSystem = (int) referenceSystems.size();
		}
		referenceSystems.push_back(str.c_str());
	}

	if (currentReferenceSystem == -1 && CalCtx.referenceTrackingSystem == "")
	{
		if (CalCtx.state == CalibrationState::ContinuousStandby) {
			auto iter = std::find(state.trackingSystems.begin(), state.trackingSystems.end(), CalCtx.referenceStandby.trackingSystem);
			if (iter != state.trackingSystems.end()) {
				currentReferenceSystem = iter - state.trackingSystems.begin();
			}
		}
		else {
			currentReferenceSystem = firstReferenceSystemNotTargetSystem;
		}
	}

	ImGui::PushItemWidth(paneWidth);
	ImGui::Combo("##ReferenceTrackingSystem", &currentReferenceSystem, &referenceSystems[0], (int) referenceSystems.size());

	if (currentReferenceSystem != -1 && currentReferenceSystem < (int) referenceSystems.size())
	{
		CalCtx.referenceTrackingSystem = std::string(referenceSystems[currentReferenceSystem]);
		if (CalCtx.referenceTrackingSystem == CalCtx.targetTrackingSystem)
			CalCtx.targetTrackingSystem = "";
	}

	if (CalCtx.targetTrackingSystem == "") {
		if (CalCtx.state == CalibrationState::ContinuousStandby) {
			auto iter = std::find(state.trackingSystems.begin(), state.trackingSystems.end(), CalCtx.targetStandby.trackingSystem);
			if (iter != state.trackingSystems.end()) {
				currentTargetSystem = iter - state.trackingSystems.begin();
			}
		}
		else {
			currentTargetSystem = 0;
		}
	}

	std::vector<const char *> targetSystems;
	for (auto &str : state.trackingSystems)
	{
		if (str != CalCtx.referenceTrackingSystem)
		{
			if (str != "" && str == CalCtx.targetTrackingSystem)
				currentTargetSystem = (int) targetSystems.size();
			targetSystems.push_back(str.c_str());
		}
	}

	ImGui::SameLine();
	ImGui::Combo("##TargetTrackingSystem", &currentTargetSystem, &targetSystems[0], (int) targetSystems.size());

	if (currentTargetSystem != -1 && currentTargetSystem < targetSystems.size())
	{
		CalCtx.targetTrackingSystem = std::string(targetSystems[currentTargetSystem]);
	}

	ImGui::PopItemWidth();
}

void AppendSeparated(std::string &buffer, const std::string &suffix)
{
	if (!buffer.empty())
		buffer += " | ";
	buffer += suffix;
}

std::string LabelString(const VRDevice &device)
{
	std::string label;

	/*if (device.controllerRole == vr::TrackedControllerRole_LeftHand)
		label = "Left Controller";
	else if (device.controllerRole == vr::TrackedControllerRole_RightHand)
		label = "Right Controller";
	else if (device.deviceClass == vr::TrackedDeviceClass_Controller)
		label = "Controller";
	else if (device.deviceClass == vr::TrackedDeviceClass_HMD)
		label = "HMD";
	else if (device.deviceClass == vr::TrackedDeviceClass_GenericTracker)
		label = "Tracker";*/

	AppendSeparated(label, device.model);
	AppendSeparated(label, device.serial);
	return label;
}

std::string LabelString(const StandbyDevice& device) {
	std::string label("< ");

	label += device.model;
	AppendSeparated(label, device.serial);

	label += " >";
	return label;
}

void BuildDeviceSelection(const VRState &state, int &initialSelected, const std::string &system, StandbyDevice &standbyDevice)
{
	int selected = initialSelected;
	ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), u8"�豸���ͣ� %s", system.c_str());

	if (selected != -1)
	{
		bool matched = false;
		for (auto &device : state.devices)
		{
			if (device.trackingSystem != system)
				continue;

			if (selected == device.id)
			{
				matched = true;
				break;
			}
		}

		if (!matched)
		{
			// Device is no longer present.
			selected = -1;
		}
	}

	bool standby = CalCtx.state == CalibrationState::ContinuousStandby;

	if (selected == -1 && !standby)
	{
		for (auto &device : state.devices)
		{
			if (device.trackingSystem != system)
				continue;

			if (device.controllerRole == vr::TrackedControllerRole_LeftHand)
			{
				selected = device.id;
				break;
			}
		}

		if (selected == -1) {
			for (auto& device : state.devices)
			{
				if (device.trackingSystem != system)
					continue;
				
				selected = device.id;
				break;
			}
		}
	}

	if (selected == -1 && standby) {
		bool present = false;
		for (auto& device : state.devices)
		{
			if (device.trackingSystem != system)
				continue;

			if (standbyDevice.model != device.model) continue;
			if (standbyDevice.serial != device.serial) continue;

			present = true;
			break;
		}

		if (!present) {
			auto label = LabelString(standbyDevice);
			ImGui::Selectable(label.c_str(), true);
		}
	}

	for (auto &device : state.devices)
	{
		if (device.trackingSystem != system)
			continue;

		auto label = LabelString(device);
		if (ImGui::Selectable(label.c_str(), selected == device.id)) {
			selected = device.id;
		}
	}
	if (selected != initialSelected) {
		const auto& device = std::find_if(state.devices.begin(), state.devices.end(), [&](const auto& d) { return d.id == selected; });
		if (device == state.devices.end()) return;

		initialSelected = selected;
		standbyDevice.trackingSystem = system;
		standbyDevice.model = device->model;
		standbyDevice.serial = device->serial;
	}
}

void BuildDeviceSelections(const VRState &state)
{
	ImGuiStyle &style = ImGui::GetStyle();
	ImVec2 paneSize(ImGui::GetWindowContentRegionWidth() / 2 - style.FramePadding.x, ImGui::GetTextLineHeightWithSpacing() * 5 + style.ItemSpacing.y * 4);

	ImGui::BeginChild("left device pane", paneSize, true);
	BuildDeviceSelection(state, CalCtx.referenceID, CalCtx.referenceTrackingSystem, CalCtx.referenceStandby);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("right device pane", paneSize, true);
	BuildDeviceSelection(state, CalCtx.targetID, CalCtx.targetTrackingSystem, CalCtx.targetStandby);
	ImGui::EndChild();

	if (ImGui::Button(u8"��ʶѡ�����豸��LED��˸���񶯣�", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetTextLineHeightWithSpacing() + 4.0f)))
	{
		for (unsigned i = 0; i < 100; ++i)
		{
			vr::VRSystem()->TriggerHapticPulse(CalCtx.targetID, 0, 2000);
			vr::VRSystem()->TriggerHapticPulse(CalCtx.referenceID, 0, 2000);
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
}

VRState LoadVRState() {
	VRState state = VRState::Load();
	auto& trackingSystems = state.trackingSystems;

	// Inject entries for continuous calibration targets which have yet to load

	if (CalCtx.state == CalibrationState::ContinuousStandby) {
		auto existing = std::find(trackingSystems.begin(), trackingSystems.end(), CalCtx.referenceTrackingSystem);
		if (existing == trackingSystems.end()) {
			trackingSystems.push_back(CalCtx.referenceTrackingSystem);
		}

		existing = std::find(trackingSystems.begin(), trackingSystems.end(), CalCtx.targetTrackingSystem);
		if (existing == trackingSystems.end()) {
			trackingSystems.push_back(CalCtx.targetTrackingSystem);
		}
	}

	return state;
}

void BuildProfileEditor()
{
	ImGuiStyle &style = ImGui::GetStyle();
	float width = ImGui::GetWindowContentRegionWidth() / 3.0f - style.FramePadding.x;
	float widthF = width - style.FramePadding.x;

	TextWithWidth("YawLabel", "Yaw", width);
	ImGui::SameLine();
	TextWithWidth("PitchLabel", "Pitch", width);
	ImGui::SameLine();
	TextWithWidth("RollLabel", "Roll", width);

	ImGui::PushItemWidth(widthF);
	ImGui::InputDouble("##Yaw", &CalCtx.calibratedRotation(1), 0.1, 1.0, "%.8f");
	ImGui::SameLine();
	ImGui::InputDouble("##Pitch", &CalCtx.calibratedRotation(2), 0.1, 1.0, "%.8f");
	ImGui::SameLine();
	ImGui::InputDouble("##Roll", &CalCtx.calibratedRotation(0), 0.1, 1.0, "%.8f");

	TextWithWidth("XLabel", "X", width);
	ImGui::SameLine();
	TextWithWidth("YLabel", "Y", width);
	ImGui::SameLine();
	TextWithWidth("ZLabel", "Z", width);

	ImGui::InputDouble("##X", &CalCtx.calibratedTranslation(0), 1.0, 10.0, "%.8f");
	ImGui::SameLine();
	ImGui::InputDouble("##Y", &CalCtx.calibratedTranslation(1), 1.0, 10.0, "%.8f");
	ImGui::SameLine();
	ImGui::InputDouble("##Z", &CalCtx.calibratedTranslation(2), 1.0, 10.0, "%.8f");

	TextWithWidth("ScaleLabel", "Scale", width);

	ImGui::InputDouble("##Scale", &CalCtx.calibratedScale, 0.0001, 0.01, "%.8f");
	ImGui::PopItemWidth();
}

void TextWithWidth(const char *label, const char *text, float width)
{
	ImGui::BeginChild(label, ImVec2(width, ImGui::GetTextLineHeightWithSpacing()));
	ImGui::Text(text);
	ImGui::EndChild();
}
