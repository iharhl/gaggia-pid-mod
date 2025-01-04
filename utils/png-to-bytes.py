from PIL import Image
import numpy as np

# Load the grayscale image
image_path = "docs/display-img/therm-v2.png"

with Image.open(image_path) as img:
    img = img.convert("L")  # grayscale mode
    raw_data = np.array(img)  # Convert to NumPy array (2D array of shape 32x32)

# Convert to 1D byte array
byte_array = raw_data.flatten().tobytes()

# Verify size and preview some bytes
print(f"Byte array size: {len(byte_array)} bytes")  # Should be 1024
print(f"Mid 10 bytes: {byte_array[500:550]}")

# Convert the byte array back to image
reconstr_image = np.frombuffer(byte_array, dtype=np.uint8).reshape((32, 32))

# Create and save the reconstructed image
reconstr_image = Image.fromarray(reconstr_image, mode="L")
reconstr_image.save("reconstr_image.png")
