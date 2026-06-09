from PIL import Image, ImageDraw

# size.ico olustur (Olcu ikonu - Cetvel resmi)
img_size = Image.new('RGBA', (16, 16), (255, 255, 255, 0))
draw = ImageDraw.Draw(img_size)
# Mavi bir cetvel/olcu cizgisi cizelim
draw.rectangle([1, 4, 14, 11], fill=(41, 128, 185), outline=(31, 97, 141))
draw.line([3, 4, 3, 7], fill=(255, 255, 255))
draw.line([6, 4, 6, 7], fill=(255, 255, 255))
draw.line([9, 4, 9, 7], fill=(255, 255, 255))
draw.line([12, 4, 12, 7], fill=(255, 255, 255))
img_size.save('size.ico', format='ICO')

# color.ico olustur (Renk ikonu - Renk paleti resmi)
img_color = Image.new('RGBA', (16, 16), (255, 255, 255, 0))
draw = ImageDraw.Draw(img_color)
# Uc adet renkli daire cizelim (RGB)
draw.ellipse([1, 1, 9, 9], fill=(231, 76, 60)) # Kirmizi
draw.ellipse([7, 1, 15, 9], fill=(46, 204, 113)) # Yesil
draw.ellipse([4, 6, 12, 14], fill=(52, 152, 219)) # Mavi
img_color.save('color.ico', format='ICO')

print("Ikonlar basariyla olusturuldu!")
