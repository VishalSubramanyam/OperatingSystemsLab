while read n || [[ "$n" ]]; do
  for ((i=2;i<=n;i++)); do arr[i]=$i; done;
  for ((i=2;i<=n;i++)); do
    if [[ ${arr[i]} -ne 0 ]]; then
      if [[ $(($n % $i)) -eq 0 ]]; then
        echo -n $i >> output.txt
        echo -n " " >> output.txt 
      fi
      for ((j=i*i;j<=n;j+=i)); do
        arr[j]=0
      done
    fi
  done
  echo "" >> output.txt
done < input.txt