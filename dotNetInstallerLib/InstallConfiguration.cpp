#include "StdAfx.h"
#include "Configuration.h"
#include "InstallerLog.h"
#include "InstallerSession.h"
#include "InstallConfiguration.h"
#include "MsiComponent.h"
#include "MsuComponent.h"
#include "CmdComponent.h"
#include "OpenFileComponent.h"

InstallConfiguration::InstallConfiguration()
	: Configuration(configuration_install)
    , must_reboot_required(false)
    , auto_close_if_installed(false)
    , auto_close_on_error(false)
    , dialog_show_installed(false)
    , dialog_show_required(false)
    , allow_continue_on_error(true)
    , log_enabled(false)
{

}

void InstallConfiguration::Load(TiXmlElement * node)
{
	CHECK_BOOL(node != NULL,
		L"Expected 'configuration' node");

	CHECK_BOOL(0 == strcmp(node->Value(), "configuration"),
		L"Expected 'configuration' node, got '" << DVLib::string2wstring(node->Value()) << L"'");

	Configuration::Load(node);

    // auto-enabled log options
    log_enabled = DVLib::wstring2bool(DVLib::UTF8string2wstring(node->Attribute("log_enabled")), false);
    log_file = DVLib::UTF8string2wstring(node->Attribute("log_file"));
    // defines where to extract files and auto-delete options
    cab_path = DVLib::UTF8string2wstring(node->Attribute("cab_path"));
	InstallerSession::s_sessioncabpath = cab_path;
    cab_path_autodelete = DVLib::wstring2bool(DVLib::UTF8string2wstring(node->Attribute("cab_path_autodelete")), true);
    // positions within the dialog
    dialog_position.FromString(DVLib::UTF8string2wstring(node->Attribute("dialog_position")));
    dialog_components_list_position.FromString(DVLib::UTF8string2wstring(node->Attribute("dialog_components_list_position")));
    dialog_message_position.FromString(DVLib::UTF8string2wstring(node->Attribute("dialog_message_position")));
    dialog_bitmap_position.FromString(DVLib::UTF8string2wstring(node->Attribute("dialog_bitmap_position")));
    dialog_otherinfo_link_position.FromString(DVLib::UTF8string2wstring(node->Attribute("dialog_otherinfo_link_position")));
    dialog_osinfo_position.FromString(DVLib::UTF8string2wstring(node->Attribute("dialog_osinfo_position")));
    dialog_install_button_position.FromString(DVLib::UTF8string2wstring(node->Attribute("dialog_install_button_position")));
    dialog_cancel_button_position.FromString(DVLib::UTF8string2wstring(node->Attribute("dialog_cancel_button_position")));
    dialog_skip_button_position.FromString(DVLib::UTF8string2wstring(node->Attribute("dialog_skip_button_position")));
    // other dialog options
	cancel_caption = DVLib::UTF8string2wstring(node->Attribute("cancel_caption"));
	dialog_bitmap = InstallerSession::MakePath(DVLib::UTF8string2wstring(node->Attribute("dialog_bitmap")));
	dialog_caption = DVLib::UTF8string2wstring(node->Attribute("dialog_caption"));
	dialog_message = DVLib::UTF8string2wstring(node->Attribute("dialog_message"));
	skip_caption = DVLib::UTF8string2wstring(node->Attribute("skip_caption"));
	install_caption = DVLib::UTF8string2wstring(node->Attribute("install_caption"));
	status_installed = DVLib::UTF8string2wstring(node->Attribute("status_installed"));
	status_notinstalled = DVLib::UTF8string2wstring(node->Attribute("status_notinstalled"));
	failed_exec_command_continue = DVLib::UTF8string2wstring(node->Attribute("failed_exec_command_continue"));
	installation_completed = DVLib::UTF8string2wstring(node->Attribute("installation_completed"));
	installation_none = DVLib::UTF8string2wstring(node->Attribute("installation_none"));
	reboot_required = DVLib::UTF8string2wstring(node->Attribute("reboot_required"));
    must_reboot_required = DVLib::wstring2bool(DVLib::UTF8string2wstring(node->Attribute("must_reboot_required")), false);
	installing_component_wait = DVLib::UTF8string2wstring(node->Attribute("installing_component_wait"));
	dialog_otherinfo_caption = DVLib::UTF8string2wstring(node->Attribute("dialog_otherinfo_caption"));
	dialog_otherinfo_link = InstallerSession::MakePath(DVLib::UTF8string2wstring(node->Attribute("dialog_otherinfo_link")));
	// completion commands
	complete_command = InstallerSession::MakePath(DVLib::UTF8string2wstring(node->Attribute("complete_command")));
	complete_command_silent = InstallerSession::MakePath(DVLib::UTF8string2wstring(node->Attribute("complete_command_silent")));
	auto_close_if_installed = DVLib::wstring2bool(DVLib::UTF8string2wstring(node->Attribute("auto_close_if_installed")), true);
    auto_close_on_error = DVLib::wstring2bool(DVLib::UTF8string2wstring(node->Attribute("auto_close_on_error")), false);
    allow_continue_on_error = DVLib::wstring2bool(DVLib::UTF8string2wstring(node->Attribute("allow_continue_on_error")), true);
    dialog_show_installed = DVLib::wstring2bool(DVLib::UTF8string2wstring(node->Attribute("dialog_show_installed")), true);
    dialog_show_required = DVLib::wstring2bool(DVLib::UTF8string2wstring(node->Attribute("dialog_show_required")), true);
    // message and caption to show during CAB extraction
    cab_dialog_caption = DVLib::UTF8string2wstring(node->Attribute("cab_dialog_caption"));
    cab_dialog_message = DVLib::UTF8string2wstring(node->Attribute("cab_dialog_message"));
    cab_cancelled_message = DVLib::UTF8string2wstring(node->Attribute("cab_cancelled_message"));
	// components
	TiXmlNode * child = NULL;
	while ( (child = node->IterateChildren("component", child)) != NULL)
	{
		TiXmlElement * node_component = child->ToElement();
		if (node_component == NULL)
			continue;

		std::wstring component_type = DVLib::UTF8string2wstring(node_component->Attribute("type"));

		shared_any<Component *, close_delete> component;
		if (component_type == L"msi") component = shared_any<Component *, close_delete>(new MsiComponent());
		else if (component_type == L"msu") component = shared_any<Component *, close_delete>(new MsuComponent());
		else if (component_type == L"cmd") component = shared_any<Component *, close_delete>(new CmdComponent());
		else if (component_type == L"openfile") component = shared_any<Component *, close_delete>(new OpenFileComponent());
		else 
		{
			THROW_EX(L"Unsupported component type: " << component_type);
		}

		component->Load(node_component);
		components.add(component);
	}

	LOG(L"Loaded " << components.size() << L" component(s) from configuration type=" << type 
		<< L" (lcid=" << lcid
		<< L", os_filter_greater=" << os_filter_greater
		<< L", os_filter_smaller=" << os_filter_smaller
		<< L", processor_architecture_filter=" << processor_architecture_filter
		<< L")");
}

Components InstallConfiguration::GetSupportedComponents(DVLib::LcidType lcidtype) const
{
	return components.GetSupportedComponents(lcidtype);
}
