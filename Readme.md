# Editor Screenshot

This Unreal plugin allows you to take screenshots of editor windows based on a description file.
Its use case is primarily for tool developers who want to automate creating screenshots of their tools.

The goal is to reproduce solid screenshots for documentation purposes that can deal with changes in the tool's UI.

## Usage

To capture a screenshot, run the `EditorScreenshot.Capture <File>` command.
This command expects a path to a description file, which is an INI file. The format of the file is described in the following section.

The path to the file is relative to the project directory, or an absolute path.

The resulting screenshots are saved to the `(Project)/Saved/EditorScreenshots/` directory.
A subfolder is created based on the name of the description file.

## Description file format

The description file is an INI file.
Each section describes a single window to capture.
The section name is a path to a window, which follows the hierarchy of tab managers, beginning at the global tab manager.

Each section contains the following keys:

- `Size`: The size of the window to capture, in pixels, using the format `WxH`.
- `Highlight`: A path to a child widget to highlight (See next section). This key can appear multiple times.

### Widget highlighting

The path is a list of path elements separated by a dot.
Each element is either a widget type name or an index.
If no index is specified, the first widget of that type is used.

In deeply nested widgets, the tool does a deep search for a widget when the widget type is specified within angle brackets `[]`.

#### Example

- `Highlight=SVerticalBox.SHorizontalBox.3.SButton.2`: Highlight the third button in the fourth horizontal box in the first vertical box.
- `Highlight=[SMyWidget].SHorizontalBox.STextBlock`: Do a deep search for a widget of type `SMyWidget`, then find the first text block in the first horizontal box.

### Example description file

This example captures two screenshots:

- An overview screenshot of the tab `MyEditorWindow` in `1920x1080`.
- A detail screenshot of the `DetailsTab`, which is registered as a tab in the `MyEditorWindow` tab manager.
  This screenshot will have a resolution of `500x800` and have a button and a text block highlighted.

When the file is named `MyEditorWindow.ini`, the resulting screenshots will be saved to:

- `(Project)/Saved/EditorScreenshots/MyEditorWindow/MyEditorWindow.png`
- `(Project)/Saved/EditorScreenshots/MyEditorWindow/DetailsTab.png`

```ini
[MyEditorWindow]
Size=1920x1080

[MyEditorWindow/DetailsTab]
Size=500x800
Highlight=SVerticalBox.SHorizontalBox.SButton.2
Highlight=[SCustomStructVisualizer].SVerticalBox.STextBlock
```

## Trivia

This plugin has absolutely no UI.
