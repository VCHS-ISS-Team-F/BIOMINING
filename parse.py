data = """
947463712-cnt=264Ph=156mV=184980
947463717-cnt=265Ph=173mV=184529
947463722-cnt=266Ph=173mV=184529
947463727-cnt=267Ph=144mV=185303
947463732-cnt=268Ph=178mV=184400
947463737-cnt=269Ph=169mV=184658
947463742-cnt=270Ph=149mV=185174
947463747-cnt=271Ph=169mV=184658
947463753-cnt=272Ph=169mV=184658
947463758-cnt=273Ph=156mV=184980
947463763-cnt=274Ph=169mV=184658
"""
for i, line in enumerate(data.splitlines()):
    if i == 0:
        continue
    (timestamp, data) = line.split("-", 1)
    print(int(data.split("=")[-1]) / 100)

# print(data)
