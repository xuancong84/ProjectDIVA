# ProjectDIVA
Project DIVA PC HD

Current version is 3.5.1:

1. when the combo gauge is full, press Shift to accelerate; however, if break, speed falls back to normal.

2. added UTF-8 mult-language support, add or modify text files in ./locale/*.txt to add a new language or edit string for an existing language. Currently, all other languages besides English and Chinese are obtained via Google Translate.

3. added multiple font support.


For game players:
- copy everything except the ./src folder
- copy music packs from the Internet elsewhere


For developers:
- copy everything including the ./src folder
- please set library and include paths for bass, dxsdk and FMOD-sound-system in ./src/external folder.
- all commits without any description are minor/trivial modifications such as editing description, fixing spelling errors, etc.


Key programming techniques include how to:
- play audio (using bass library and FMOD-sound-system) and video (using windows media library) at arbituary speed while maintaining synchrony.
- use DirectDraw to draw point sprite, perform scaling
- use Direct3D to draw objects, lighting, perform object-to-screen, screen-to-object coordinate conversion, ray-triangle intersection detection
- use DirectX to draw UTF-8 characters, manipulate UTF-8 strings and filenames in Win32 API
- enumerate display resolutions and change display resolution
- enumerate system font and change DirectX font
- enumerate system locales
- use Window keyhook to capture keyboard input


Acknowledgement:
This work was originally created by the Chinese GameMaster Studio group (http://gamemastercn.com/).
It was later modified and enhanced by me (Wang Xuancong) for learning Window API, multimedia and computer graphics programming.
I will now release its source code to the open public to continue its development and enhancement.