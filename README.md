# DirectoryOpus-CSV-viewer-plugin
A viewer plugin for Directory Opus that displays CSV files as a table.
![Screenshot](doc/screenshot.png)

# Features
* Automatically detects the `,`, `;`, `\t`, `|`, `^` delimiter.
* Automatically detects if the first row is a header.
* Supported encodings: `UTF-8`, `UTF-8 BOM`, `UTF-16LE`, `UTF-16BE`
* Displays only the first 1MB to 10MB of the file content, depending on the configuration.
* Selected rows can be copied to the clipboard by pressing `Ctrl+C`.
* Configurable options (max "Read Size" is 10 MB)<br>
<img src="doc/screenshot_config_dialog.png" width="25%"/>

# How to use
* Download the zipped `DLL` file from the [releases page](https://github.com/PolarGoose/DirectoryOpus-CSV-viewer-plugin/releases)
* Copy the `DLL` file to `C:\Program Files\GPSoftware\Directory Opus\Viewers` folder
* Restart Directory Opus
* Every time you select a `.csv` file, the plugin will be used to show the content in the Viewer Pane.

# References
* Discussion of this plugin on the Directory Opus forum: [CSV viewer 2](https://resource.dopus.com/t/csv-viewer-2/56568)
