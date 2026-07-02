#!/usr/bin/env bash
set -euo pipefail

DIR1="sample_snush_result"
DIR2="snush_result"

# 존재하지 않는 디렉터리 체크
if [[ ! -d $DIR1 || ! -d $DIR2 ]]; then
  echo "[Error] 결과 디렉터리가 없습니다: $DIR1 또는 $DIR2"
  exit 1
fi

echo "=== Comparing $DIR1 vs $DIR2 ==="
echo

# 요약 파일
SUMMARY="diff_summary.txt"
> "$SUMMARY"

# DIR1 의 각 파일에 대해
for f1 in "$DIR1"/*; do
  name=$(basename "$f1")
  f2="$DIR2/$name"

  if [[ ! -e $f2 ]]; then
    echo "[MISSING] $name exists in $DIR1 but not in $DIR2" | tee -a "$SUMMARY"
    continue
  fi

  # 실제 diff 실행
  if diff -u "$f1" "$f2" > "diff_${name}.patch"; then
    echo "[OK]       $name identical" | tee -a "$SUMMARY"
    rm "diff_${name}.patch"
  else
    echo "[DIFF]     $name differs (see diff_${name}.patch)" | tee -a "$SUMMARY"
  fi
done

echo
echo "Done. Summary in $SUMMARY, per‐file diffs in diff_<filename>.patch"
