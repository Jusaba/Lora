nLinea=$(grep "#define VBuild" -n ServerPic.h | cut -d ":" -f 1 )
sed -i "${nLinea} d" ServerPic.h
sed -i "${nLinea} s|^|\t#define VBuild  \"$1\"\n|" ServerPic.h
git config --global "user.name" "Teamcity"
git config --global "user.email" "jsbpublico@gmail.com"
git add ServerPic.h
git commit -m "[skip] Compilacion version "$1
git push "https://"$2"@github.com/jusaba/LoraTest" HEAD:master
mkdir librerias
git clone "https://"$2"@github.com/jusaba/LibreriasServerpic" librerias
sh boardlist.sh "$3" "$1"
rm -rf librerias
rm -f *
