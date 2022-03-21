[![DOI](https://zenodo.org/badge/370571978.svg)](https://zenodo.org/badge/latestdoi/370571978)

Repository for the HC (Harmonic Compression) and PC (Pseudo cycles based compression) codecs. Since the codebases for both formats share a considerable portion of the code (.wav reading, saving, segmentation into pseudo cycles, etc), the authors decided to unify development in a single repository, and release a single executable for both codecs.

# Installation

The executable for Windows OS 64 bits can be downloaded in the releases section, along with the necessary libsndfile-1.dll, as a .zip archive. Those can be unpacked to a convenient folder and referenced via relative paths, or manually added to the system path.

For other OSes, it should be possible to compile the source code via the Visual Studio solution. Not all C++ dependencies are included in the repository, however: only those that are pointed by relative paths, starting with "$(SolutionDir)".

# Usage

Run the executable with the flag -h or --help

-[a string] [-csv] [path/to/file_1.wav]...[path/to/file_n.wav]  [path/to/file_1.hc]...[path/to/file_m.hc]
-a or --append: (default "_reconstructed") string to be appended to each reconstructed file name
-q or --quality: 0.0 <= q <= 1.0. If supplied, .pc mode is used, with quality q
-csv: flag to turn on saving csv files with the beginning of each pseudo cycle, envelope and average waveform
If no path is given, the root folder will be scanned for .wav and .hc files, and those will be processed accordingly

<!-- # Papers -->

<!-- The generated  -->

## Harmonic Compression

Scripts 001* to 003* are related to this method, and generate the data available at "004_results_HC".

Results can be heard at [harmoniccompression.firebaseapp.com](https://harmoniccompression.firebaseapp.com/) or [harmoniccompression.web.app](https://harmoniccompression.web.app/)

<!-- ## Pseudo cycles based compression -->

<!-- TODO site .pc-->

## Relevant links

- [Lame](https://lame.sourceforge.io/)
- [FFmpeg](https://www.ffmpeg.org/)
- [MPEG group](https://mpeg.chiariglione.org/)
- [Opus codec](https://opus-codec.org/)
- [AAC standard](https://www.iso.org/standard/43345.html)
- [MP3 standard](https://www.iso.org/standard/22412.html)
- [FITPACK](https://www.netlib.org/fitpack/)
- [fitpackpp](https://github.com/jbaayen/fitpackpp)

## Sources of the unprocessed samples

- [VocalSet](https://zenodo.org/record/1442513)
- [Human Voice Dataset](https://github.com/vocobox/human-voice-dataset)
- [Philharmonia](https://philharmonia.co.uk/resources/sound-samples/)
- [University of Iowa Electronic Music Studios](http://theremin.music.uiowa.edu/MIS.html)

## Additional tools

- [Plotly](https://plotly.com/)
- [Libsndfile](http://www.mega-nerd.com/libsndfile/)
- [Wavesurfer-js](https://wavesurfer-js.org/)
- [Pandoc](https://pandoc.org/)
- [MPEG-7 audio encoder](http://mpeg7audioenc.sourceforge.net/)