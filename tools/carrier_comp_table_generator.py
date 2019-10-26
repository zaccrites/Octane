
MAX_CARRIERS = 8

for i in range(MAX_CARRIERS):
    num_carriers = i + 1
    compensation_factor = round(0x7fff / num_carriers)
    print(f"3'd{i}: 16'h{compensation_factor:04x}")
