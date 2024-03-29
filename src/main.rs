use std::fs;

use anyhow::Result;
use firered_save_editor::{Gender, Save};
use winsafe::{
    co, gui, prelude::*, CoCreateInstance, CoInitializeEx, IFileOpenDialog,
    MenuItem, ACCEL, HACCEL, HMENU,
};

// The values for command IDs are arbitrary.
const FILE_OPEN_COMMAND_ID: u16 = 0;

struct EditorWindow {
    window: gui::WindowControl,
    gender_radio_group: gui::RadioGroup,
}

impl EditorWindow {
    fn new(parent: &gui::WindowMain) -> EditorWindow {
        let mut options: gui::WindowControlOpts = Default::default();
        options.style = options.style & !co::WS::VISIBLE;
        let window = gui::WindowControl::new(parent, options);

        let _ = gui::Label::new(
            &window,
            gui::LabelOpts {
                text: String::from("Gender"),
                position: (20, 20),
                ..Default::default()
            },
        );

        let gender_radio_group = gui::RadioGroup::new(
            &window,
            &[
                gui::RadioButtonOpts {
                    text: "Boy".to_owned(),
                    position: (20, 40),
                    ..Default::default()
                },
                gui::RadioButtonOpts {
                    text: "Girl".to_owned(),
                    position: (20, 60),
                    ..Default::default()
                },
            ],
        );

        EditorWindow { window, gender_radio_group }
    }

    fn load(&self, buf: Vec<u8>) {
        let save = Save::new(buf).unwrap();

        let gender = save.player_gender();
        let gender_radio_index = match gender {
            Gender::Boy => 0,
            Gender::Girl => 1,
        };
        let gender_radio_button = &self.gender_radio_group[gender_radio_index];
        gender_radio_button.select(true);

        self.window.hwnd().ShowWindow(co::SW::SHOW);
    }
}

fn main() -> Result<()> {
    let file_submenu = HMENU::CreatePopupMenu()?;
    file_submenu.append_item(&[MenuItem::Entry(
        FILE_OPEN_COMMAND_ID,
        "Open\tCtrl+O",
    )])?;

    let menu = HMENU::CreateMenu()?;
    menu.append_item(&[MenuItem::Submenu(&file_submenu, "File")])?;

    let accel_table = HACCEL::CreateAcceleratorTable(&mut [ACCEL {
        fVirt: co::ACCELF::VIRTKEY | co::ACCELF::CONTROL,
        cmd: FILE_OPEN_COMMAND_ID,
        key: co::VK::CHAR_O,
    }])?;

    let main_window = gui::WindowMain::new(gui::WindowMainOpts {
        title: String::from("Firered Save Editor"),
        size: (400, 200),
        menu,
        accel_table: Some(accel_table),
        ..Default::default()
    });

    let editor_window = EditorWindow::new(&main_window);

    main_window.on().wm_command_accel_menu(FILE_OPEN_COMMAND_ID, {
        let main_window = main_window.clone();
        move || {
            let file_dialog = CoCreateInstance::<IFileOpenDialog>(
                &co::CLSID::FileOpenDialog,
                None,
                co::CLSCTX::INPROC_SERVER,
            )?;
            // Only accept file system items.
            file_dialog.SetOptions(
                file_dialog.GetOptions()? | co::FOS::FORCEFILESYSTEM,
            )?;

            if file_dialog.Show(main_window.hwnd())? {
                let item = file_dialog.GetResult()?;
                let file_path = item.GetDisplayName(co::SIGDN::FILESYSPATH)?;

                let buf = fs::read(file_path)?;
                editor_window.load(buf);
            }

            Ok(())
        }
    });

    let _com = CoInitializeEx(
        co::COINIT::APARTMENTTHREADED | co::COINIT::DISABLE_OLE1DDE,
    )?;

    main_window.run_main(None).unwrap();

    Ok(())
}
