#!/bin/sh

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

MAGNACHAIND=${MAGNACHAIND:-$SRCDIR/magnachaind}
MAGNACHAINCLI=${MAGNACHAINCLI:-$SRCDIR/magnachain-cli}
MAGNACHAINTX=${MAGNACHAINTX:-$SRCDIR/magnachain-tx}
MAGNACHAINQT=${MAGNACHAINQT:-$SRCDIR/qt/magnachain-qt}

[ ! -x $MAGNACHAIND ] && echo "$MAGNACHAIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
BTCVER=($($MAGNACHAINCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for magnachaind if --version-string is not set,
# but has different outcomes for magnachain-qt and magnachain-cli.
echo "[COPYRIGHT]" > footer.h2m
$MAGNACHAIND --version | sed -n '1!p' >> footer.h2m

for cmd in $MAGNACHAIND $MAGNACHAINCLI $MAGNACHAINTX $MAGNACHAINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BTCVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${BTCVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
