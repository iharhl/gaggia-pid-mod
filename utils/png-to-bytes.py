from PIL import Image
import numpy as np

def convertImageToBytes(image_path, output_path):
    with Image.open(image_path) as img:
        img = img.convert("L")  # grayscale mode
        raw_data = np.array(img)  # convert to numpy array (2D array of shape 32x32)

    # Rescale values from 0-255 to 0-15 (4-bit grayscale)
    scaled_data = (raw_data // 16).astype(np.uint8)

    # Pack a pair of bytes into one
    packed_data = scaled_data[:, 0::2] | (scaled_data[:, 1::2] << 4)

    # Convert to 1D byte array
    byte_array = packed_data.flatten().tobytes()

    with open(output_path, "w+") as f:
        for idx, byte in enumerate(byte_array):
            if idx % 12:
                f.write(f"0x{byte:02X}, ")
            else:
                f.write(f"\n0x{byte:02X}, ")


# for i in range(10):
#     image_path = f"docs/display-img/num-{i}-v1.png"
#     output_path = f"build/img{i}"
#     convertImageToBytes(image_path, output_path)
#
# image_path = "docs/display-img/therm-v3.png"
# output_path = f"build/img-therm"
# convertImageToBytes(image_path, output_path)