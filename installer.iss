; Inno Setup script cho Ezcel (bản C++/Qt6).
; Biên dịch: package.bat đã chạy windeployqt -> dist\Ezcel\, rồi ISCC installer.iss
; (hoặc mở bằng Inno Setup Compiler).

#define MyAppName "Ezcel"
#ifndef MyAppVersion
  #define MyAppVersion "1.47.0"
#endif
#define MyAppPublisher "EZG"
#define MyAppExeName "Ezcel.exe"

[Setup]
; AppId riêng cho dòng sản phẩm C++ (khác bản Python) — giữ cố định để cập nhật tại chỗ.
AppId={{7C2E9A14-3B6D-4F81-9E0A-2D5C8B1F4E63}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\Ezcel
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
; Cài per-user (không cần admin).
PrivilegesRequired=lowest
OutputDir=installer
OutputBaseFilename=Ezcel-Setup-{#MyAppVersion}
SetupIconFile=assets\icon.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Tạo lối tắt ngoài Desktop"; GroupDescription: "Lối tắt:"
Name: "contextmenu"; Description: "Thêm ""Mở bằng Ezcel"" vào menu chuột phải cho .csv/.xlsx"; GroupDescription: "Tích hợp:"; Flags: unchecked

[Files]
; Toàn bộ thư mục portable do windeployqt dựng.
Source: "dist\Ezcel\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Gỡ {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; Mục "Mở bằng Ezcel" trên menu chuột phải (KHÔNG đổi app mặc định -> không đụng Excel).
Root: HKCU; Subkey: "Software\Classes\SystemFileAssociations\.csv\shell\Ezcel"; \
    ValueType: string; ValueData: "Mở bằng Ezcel"; Tasks: contextmenu; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\SystemFileAssociations\.csv\shell\Ezcel\command"; \
    ValueType: string; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: contextmenu
Root: HKCU; Subkey: "Software\Classes\SystemFileAssociations\.xlsx\shell\Ezcel"; \
    ValueType: string; ValueData: "Mở bằng Ezcel"; Tasks: contextmenu; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\SystemFileAssociations\.xlsx\shell\Ezcel\command"; \
    ValueType: string; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: contextmenu

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Chạy {#MyAppName} ngay"; \
    Flags: nowait postinstall skipifsilent
