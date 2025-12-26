#!/bin/bash

declare -A l
l["en"]="en_US"
l["ru"]="ru_RU"
l["ja"]="ja_JP"
l["zh_CN"]="zh_CN"
l["zh_TW"]="zh_TW"
l["vi"]="vi_VN"
l["es"]="es_ES"
l["fr"]="fr_FR"
l["de"]="de_DE"
l["ko"]="ko_KR"
l["pt_BR"]="pt_BR"
l["it"]="it_IT"
l["pl"]="pl_PL"

mkdir -p locales

for key in ${!l[@]}
do
    echo localedef -f UTF-8 -i "${l[${key}]}" "./locales/${key}"
    localedef -f UTF-8 -i "${l[${key}]}" "./locales/${key}"
done

rm locales.tar.gz
tar -C locales -czvf locales.tar.gz .
