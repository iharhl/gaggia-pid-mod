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

def main():
    ...
    # Usages:
    # ---------------- Example 1 ----------------------
    # import glob
    # for i in range(19):
    # image_path = glob.glob("docs/display/png-v2/*")
    # for img in image_path:
    #     output_path = f"build/{img.split('/')[-1].split('.')[0]}.txt"
    #     print(output_path)
    #     convertImageToBytes(img, output_path)
    # ---------------- Example 2 ----------------------
    # image_path = "docs/display/png/letter-h.png"
    # output_path = f"build/letter-h.txt"
    # convertImageToBytes(image_path, output_path)


if __name__ == "__main__":
    main()
