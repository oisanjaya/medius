<h1 align="center">medius</h1>
<p align="center">Highly customizable Wayland quick panel</p>
<p align="center">
</p>

<p align="center"><img width="565" height="542" alt="medius1" src="https://github.com/user-attachments/assets/781486ff-da90-4368-951d-0c1019b5163c" /></p>

<p align="center">
    <a href="#">Getting Started</a> | <a href="#">Configuration</a> | <a href="#">Setup&nbsp;Showcase</a>
</p>

## About

Main reason I create this repository because I use [Niri](https://yalter.github.io/niri) and [Waybar](https://github.com/Alexays/Waybar) but I really miss a "quick settings" and they doesn't provide built-in feature like this.

But developing such app is difficult. What button should included? What icon it should show? What command does it should run? I think that decisions are very personal, so i wrote this app. Basically this app only create window/panel on [top shell layer](https://wayland.app/protocols/wlr-layer-shell-unstable-v1) and you can write your own layout and logic. So in short we provide you the easy part of creating window/widgets, and you do the hard part of deciding what inside the panel and how it going to be styled.

## Features

- Build widget layout using [KDL](https://kdl.dev/) format in configuration files.
- Every command from config files are run in separate thread to keep UI responsive.
- Every widget assigned css class to further style the panel

## Configuration

coming soon. But included in this repo inside example-config are my config files. You can try copy theese files into ~/.config/medius and see how it works

## Building

coming soon.

## Status

medius is in development state, maybe will always be, but I believe it still useful for every day use.

## Contributing

If you'd like to help with medius, you are very welcome. Go open issues or post PR.

## Inspiration

Every OS out there, of course. I use Niri and Waybar and there are no built in 'quick settings' available.

## Contact

You are welcome to contact me at oisanjaya@gmail.com
