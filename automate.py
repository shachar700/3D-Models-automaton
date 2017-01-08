"""
The base automation file. Coordinates between HLMV and image processing.
Rotates and captures pictures of a 3d model.
Load up your model in HLMV, then run this script.
"""

from __future__ import print_function
from time import sleep

from PIL.ImageGrab import grab
from win32con import SW_MAXIMIZE
from win32gui import EnumWindows, GetWindowText, SetForegroundWindow, ShowWindow

from imageprocessor import ImageProcessor
from HLMVmodel import HLMVModel

if __name__ == '__main__':
  # The cropping boundaries, as a pixel distance from the top left:
  # (left boundary, top boundary, right boundary, bottom boundary).
  crop_boundary = (1, 42, 1279, 510)
  # File name for when the image is uploaded
  item_name = 'User Darkid Test'
  number_of_images = 24 # Y rotations
  vertical_rotations = 1 # X rotations
  # Initial parameters. Mostly, you won't need to set these.
  model = HLMVModel({
    'rotation': None,
    'translation': None,
    'rotation_offset': None,
    'vertical_offset': None
    })
  ip = ImageProcessor(number_of_images, vertical_rotations)

  def enum_callback(hwnd, _):
    """
    Focus and maximise HLMV
    """
    if GetWindowText(hwnd)[:7] == 'models\\':
      SetForegroundWindow(hwnd)
      ShowWindow(hwnd, SW_MAXIMIZE)
  EnumWindows(enum_callback, [])

  white_images = []
  model.set_background(False)
  # Loops in this order to get the images in the right order.
  for y_rot in xrange(0, 360, 360//number_of_images):
    for x_rot in xrange(-15*vertical_rotations, 15*vertical_rotations+1, 15):
      model.rotate(x_rot, y_rot)
      sleep(0.02) # Wait for redraw
      white_images.append(grab().crop(crop_boundary))

  black_images = []
  model.set_background(True)
  for y_rot in xrange(0, 360, 360//number_of_images):
    for x_rot in xrange(-15*vertical_rotations, 15*vertical_rotations+1, 15):
      model.rotate(x_rot, y_rot)
      sleep(0.02) # Wait for redraw
      black_images.append(grab().crop(crop_boundary))
  model.rotate(0, 0) # Reset back to starting rotation for user

  print('Blending...' + ' '*(len(white_images) - 12) + '|')
  for (white_image, black_image) in zip(white_images, black_images):
    print('#', end='')
    ip.blend(white_image, black_image)
  print('')
  ip.stitch_and_upload(item_name + ' 3D.jpg')
