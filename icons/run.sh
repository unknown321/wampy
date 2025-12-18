#!/bin/bash
SIZE=70x70
declare -A files=(
    ["fmLeftDouble.gif"]="arrow-left-double-symbolic.svg"
    ["fmSettings.gif"]="albumfolder-properties.svg"
    ["fmLeft.gif"]="arrow-left-symbolic.svg"
    ["fmRightDouble.gif"]="arrow-right-double-symbolic.svg"
    ["fmRight.gif"]="arrow-right-symbolic.svg"
    ["fmDelete.gif"]="delete.svg"
    ["fmSave.gif"]="document-save.svg"
    ["fmRecord.gif"]="media-record-symbolic.svg"
)

for key in "${!files[@]}"; do
    echo "${files[${key}]} -> ${key}"
    convert -density 1000 -background none -resize "${SIZE}" "${files[${key}]}" "${key}"
done
