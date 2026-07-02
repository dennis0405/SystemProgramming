#!/usr/bin/env bash
set -euo pipefail

# 테스트할 명령어 목록: myprod / myspin 조합으로 20개
commands=(
  "./myprod"
  "./myprod 3"
  "./myspin 4"
  "./myspin"
  "./myprod 5 > nums.txt"
  "sort < nums.txt > sorted_nums.txt"
  "./myspin > out.log 2> err.log"
  "cat err.log"
  "./myprod 8 | grep '^[2468]'"
  "./myprod 10 | grep 5 | wc -l"
  "./myprod 10 | grep '[13579]' | sort -r > odds.txt"
  "cat odds.txt"
  "./myprod 6 | ./myspin 3"
  "./myspin 5 &"
  "./myprod 4 > bgnums.txt &"
  "./myprod 7 | grep '[02468]' > evenbg.txt &"
  "./myprod 12 | grep '[369]' | sort > three_six_nine.txt &"
  "cat < nums.txt | grep 5 > filtered5.txt"
  "./myspin | wc -l"
  "./myprod 4 | ./myspin 2"
)

# 결과 디렉터리 준비
rm -rf sample_snush_result snush_result
mkdir -p sample_snush_result snush_result

# 명령 실행 함수
run_and_capture() {
  local shell_bin=$1
  local out_dir=$2
  local i=$3
  local cmd=$4

  # 안전한 파일명 생성
  local safe=$(echo "$cmd" | tr ' /|<>&' '______')
  local log_file="${out_dir}/${i}_${safe}.txt"

  # 명령 기록
  printf "=== [%02d] %s ===\n" "$i" "$cmd" | tee "$log_file"

  # 실행 전: 리다이렉션 대상 파일들 삭제
  local tokens=($cmd)
  local rf=()
  for ((j=0; j<${#tokens[@]}; j++)); do
    case "${tokens[j]}" in
      '>'|'1>'|'2>' )
        rf+=("${tokens[j+1]}")
        ;;
    esac
  done
  for f in "${rf[@]}"; do
    rm -f "$f"
  done

  # 명령 실행
  printf "%s\n" "$cmd" | $shell_bin >> "$log_file" 2>&1

  # 생성된 리다이렉션 파일들 결과도 복사
  for f in "${rf[@]}"; do
    if [[ -e $f ]]; then
      cp "$f" "${out_dir}/${i}_${f}"
    fi
  done
}

# 메인 루프
i=0
for cmd in "${commands[@]}"; do
  i=$((i+1))
  # sample_snush
  run_and_capture "./sample_snush" "sample_snush_result" "$i" "$cmd"
  # snush
  run_and_capture "./snush"        "snush_result"        "$i" "$cmd"
done
