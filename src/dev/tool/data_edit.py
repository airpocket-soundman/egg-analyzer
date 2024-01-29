# データ、時間となっているデータをデータだけにして、データは0-1に標準化する。

import csv

data_folder_path = "C:\\Users\\yamas\\Desktop\\onsen series3\\3_15\\"

#data_folder_path = "C:\\Users\\DS1DPC2003M\\Desktop\\egg sim\\boiled_egg\\"
data_number = 200           #総データ数は200

for n in range(data_number):

    data_list = []
    file_name = data_folder_path + "0_" + str(n).zfill(3) + ".csv"
    with open(file_name,newline="") as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
        # 1列目のデータをリストに追加
            raw_data = float(row[0])
            data_list.append(raw_data)

    #正規化する

    min_value = min(data_list)
    data_list = [x - min_value for x in data_list]
    max_value = max(data_list)
    data_list = [x / max_value for x in data_list]


    file_name = data_folder_path + "r_" + str(n).zfill(3) + ".csv"
    # CSVファイルに書き込む
    file_name = data_folder_path + "r_" + str(n).zfill(3) + ".csv"
    with open(file_name, "w", newline="") as csvfile:
        writer = csv.writer(csvfile)
        for row in data_list:
            writer.writerow([row])

    print(n)
