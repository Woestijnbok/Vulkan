# Introduction
 
After making a [DirectX 11 Project](https://github.com/Woestijnbok/DirectX-11) it was time to try and recreate the same project but in Vulkan.
This project was not just about porting my renderer from DirectX to Vulkan, during the process I also tried to learn some new concepts in graphics programming.
The main shader here is a basic concept of a pbr one written in glsl, the scene has one directional light.

<div align="center">
  <img src="https://github.com/Woestijnbok/Vulkan/blob/main/Screenshots/Combined.jpg" width="auto" height="auto">
</div>

# Controls

- **Camera Position**: The camera can be moved forward via **WS**, up with **QE** and to the right using **AD** (UE Controls).
- **Camera Rotation**: Camera's pitch and yaw can change by dragging the **RIGHT MOUSE BUTTON** while it's down.
- **General**: To go back to the combined rendering mode press **1**, to stop the vehicle from rotating use **R**.
- **Other**: Using the **NUMBER KEYS** you can change the render mode: **2** Base Color, **3** Normal, **4** Glossiness and **5** for Specular

# Rendering Modes

You can view different rendering modes as explained in the controls paragraph.
Below are pictures of the different rendering modes: Base Color, Normal, Glossiness and finally Specular.
There is a screenshot of the combined rendering mode at the introduction paragraph.

<div align="center">
  <img src="https://github.com/Woestijnbok/Vulkan/blob/main/Screenshots/Base%20Color.jpg" width="400" height="auto">
 <img src="https://github.com/Woestijnbok/Vulkan/blob/main/Screenshots/Normal.jpg" width="400" height="auto">
</div>

<div align="center">
  <img src="https://github.com/Woestijnbok/Vulkan/blob/main/Screenshots/Glossiness.jpg" width="400" height="auto">
 <img src="https://github.com/Woestijnbok/Vulkan/blob/main/Screenshots/Specular.jpg" width="400" height="auto">
</div>

# Final Thoughts

This project was a good learning experience into the Vulkan world.
I enjoyed learning graphics programming concept and other concept who were more specific to Vulkan.
I hope people can use this project to learn more about graphics programming and Vulkan like I did!
