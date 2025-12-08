use std::process::{Command, Stdio};
use std::path::PathBuf;
use serde::{Deserialize, Serialize};
use tauri::Manager;
use regex::Regex;

#[derive(Debug, Serialize, Deserialize)]
pub struct CommandResult {
    success: bool,
    stdout: String,
    stderr: String,
    exit_code: i32,
    error: Option<String>,
}

/// Get the path to the bundled filevault CLI
fn get_filevault_exe_path(app_handle: &tauri::AppHandle) -> Result<PathBuf, String> {
    // Try to find the CLI in the resource directory (bundled with Tauri)
    let resource_path = app_handle
        .path()
        .resource_dir()
        .map_err(|e| format!("Failed to get resource directory: {}", e))?;

    // CLI is bundled in bin/ folder
    #[cfg(target_os = "windows")]
    let exe_name = "filevault.exe";
    #[cfg(not(target_os = "windows"))]
    let exe_name = "filevault";

    let exe_path = resource_path.join("bin").join(exe_name);

    if exe_path.exists() {
        Ok(exe_path)
    } else {
        // Fallback to development paths
        #[cfg(target_os = "windows")]
        let dev_paths = vec![
            PathBuf::from("D:\\code\\filevault\\build\\build\\Release\\bin\\release\\filevault.exe"),
            PathBuf::from("..\\..\\build\\build\\Release\\bin\\release\\filevault.exe"),
            PathBuf::from("bin\\filevault.exe"),
        ];
        #[cfg(not(target_os = "windows"))]
        let dev_paths = vec![
            PathBuf::from("../../build/build/Release/bin/release/filevault"),
            PathBuf::from("../build/build/Release/bin/release/filevault"),
            PathBuf::from("bin/filevault"),
        ];

        for dev_path in &dev_paths {
            if dev_path.exists() {
                return Ok(dev_path.clone());
            }
        }

        Err(format!("FileVault executable not found at {:?}. Tried paths: {:?}", exe_path, dev_paths))
    }
}

/// Execute FileVault CLI command
#[tauri::command]
async fn run_filevault_command(
    app_handle: tauri::AppHandle,
    args: Vec<String>,
) -> Result<CommandResult, String> {
    // Get the filevault.exe path
    let exe_path = get_filevault_exe_path(&app_handle)?;

    println!("=== FileVault Command Debug ===");
    println!("Executable: {:?}", exe_path);
    println!("Arguments count: {}", args.len());
    for (i, arg) in args.iter().enumerate() {
        println!("  Arg[{}]: {:?}", i, arg);
    }

    // Execute the command
    #[cfg(windows)]
    let output = {
        use std::os::windows::process::CommandExt;
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        
        Command::new(&exe_path)
            .args(&args)
            .stdout(Stdio::piped())
            .stderr(Stdio::piped())
            .creation_flags(CREATE_NO_WINDOW)  // Hide console window
            .output()
            .map_err(|e| format!("Failed to execute command: {}", e))?
    };
    
    #[cfg(not(windows))]
    let output = Command::new(&exe_path)
        .args(&args)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .output()
        .map_err(|e| format!("Failed to execute command: {}", e))?;
    
    println!("Exit code: {:?}", output.status.code());

    // Strip ANSI escape codes and Unicode formatting from output
    let strip_ansi = |s: &str| -> String {
        // Remove ANSI escape sequences: ESC [ ... m
        let ansi_re = Regex::new(r"\x1B\[[0-9;]*[a-zA-Z]").unwrap();
        let clean = ansi_re.replace_all(s, "");
        
        // Remove zero-width spaces and other Unicode formatting chars
        clean.chars()
            .filter(|c| !matches!(c, '\u{200B}'..='\u{200D}' | '\u{FEFF}'))
            .collect()
    };

    let mut stdout = String::from_utf8_lossy(&output.stdout).to_string();
    let mut stderr = String::from_utf8_lossy(&output.stderr).to_string();
    
    // Strip ANSI codes from both outputs
    stdout = strip_ansi(&stdout);
    stderr = strip_ansi(&stderr);
    
    let exit_code = output.status.code().unwrap_or(-1);
    let status_success = output.status.success();

    // CRITICAL FIX: CLI has bug where exit code is 0 even on failure
    // Check for error indicators in output
    let has_error_marker = stdout.contains("✗ ") || 
                          stdout.contains("Authentication failed") ||
                          stdout.contains("Wrong password") ||
                          stdout.contains("File corrupted") ||
                          stdout.contains("Failed to") ||
                          stdout.contains("Error:") ||
                          stderr.contains("error") ||
                          stderr.contains("failed");
    
    // Consider success only if exit code is 0 AND no error markers
    let success = status_success && !has_error_marker;

    let error_msg = if !success {
        // Extract error message from stdout/stderr
        let error_text = if !stderr.is_empty() {
            stderr.clone()
        } else if has_error_marker {
            // Extract error lines from stdout
            stdout.lines()
                .filter(|line| line.contains("✗ ") || line.contains("Error") || line.contains("failed"))
                .collect::<Vec<_>>()
                .join("\n")
        } else {
            format!("Command failed with exit code {}", exit_code)
        };
        Some(error_text)
    } else {
        None
    };

    Ok(CommandResult {
        success,
        stdout,
        stderr,
        exit_code,
        error: error_msg,
    })
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_shell::init())
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![run_filevault_command])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
