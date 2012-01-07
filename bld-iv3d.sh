#!/bin/sh

if ! test -d imagevis3d ; then
  iv3d="https://gforge.sci.utah.edu/svn/imagevis3d"
  env -i svn --non-interactive --trust-server-cert co ${iv3d}
fi
(cd imagevis3d && svn update)

VIS="-fvisibility=hidden"
INL="-fvisibility-inlines-hidden"
CF="-g -Wall -Wextra -O3 -U_DEBUG -DNDEBUG"
CXF="-Werror"
if test -n "${QT_BIN}" ; then
    echo "Using custom qmake..."
    qm="${QT_BIN}/qmake"
else
    qm="qmake"
fi
for d in imagevis3d ; do
  pushd ${d} &>/dev/null
    ${qm} \
        CONFIG+="release" \
        QMAKE_CONFIG="release" \
        QMAKE_CFLAGS="${VIS} ${CF}" \
        QMAKE_CXXFLAGS="${VIS} ${INL} ${CF} ${CXF}" \
        -recursive
  nice make -j8
  popd &>/dev/null
done
