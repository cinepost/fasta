#! /usr/bin/env python3.7

import fasta
from fasta import Renderer, TexFlags

renderer = Renderer()

texture_manager = renderer.textureManager()

flags = TexFlags(TexFlags.COMPRESS | TexFlags.CACHE_TO_DISK)
texture = texture_manager.textureFromImage2D("/opt/fasta/contrib/textures/mario.png", flags)
texture = texture_manager.textureFromImage2D("/opt/fasta/contrib/textures/1024x1024.jpg", flags)
texture = texture_manager.textureFromImage2D("/opt/fasta/contrib/textures/airplaneU2.bmp", flags)