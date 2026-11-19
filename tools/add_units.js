// Adds [UnitN] entries (FileName + Folder) for every C/asm source of a LoongIDE project,
// so the IDE compiles the whole project instead of just main.c.
// usage: bun add_units.js <project_dir> [name] [defines] [includes]
const fs = require('fs');
const path = require('path');

const projDir = process.argv[2];
const name = process.argv[3] || path.basename(projDir);
const defines = process.argv[4] || 'LS1B;OS_NONE';
const includes = process.argv[5] || './core/include;./core/mips;./ls1x-drv/include;./include;./libc';

const lxpPath = path.join(projDir, name + '.lxp');
if (!fs.existsSync(lxpPath)) { console.error('no lxp: ' + lxpPath); process.exit(1); }

// 1. collect sources (relative posix paths), skipping build output
function walk(dir, rel, out) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    if (e.name === 'Build' || e.name === '_obj' || e.name === '_v') continue;
    const r = rel ? rel + '/' + e.name : e.name;
    if (e.isDirectory()) walk(path.join(dir, e.name), r, out);
    else if (/\.(c|S|s|asm)$/.test(e.name)) out.push(r);
  }
  return out;
}
const files = walk(projDir, '', []).sort();

// 2. collect dirs for [Folders]
const dirs = [];
(function wdir(dir, rel) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    if (!e.isDirectory()) continue;
    if (['Build', '_obj', '_v'].includes(e.name)) continue;
    const r = rel ? rel + '/' + e.name : e.name;
    dirs.push(r);
    wdir(path.join(dir, e.name), r);
  }
})(projDir, '');
dirs.sort();

// 3. read lxp, drop existing [UnitN] and [Folders] sections, keep the rest
const raw = fs.readFileSync(lxpPath, 'utf8');
const lines = raw.split(/\r?\n/);
const kept = [];
let cur = null;
for (const line of lines) {
  const m = line.match(/^\[(.+)\]\s*$/);
  if (m) cur = m[1];
  if (cur && /^Unit\d+$/.test(cur)) continue;
  if (cur && cur === 'Folders') continue;
  if (cur && /^FoldersCount=/.test(line)) continue;
  kept.push(line);
}
// trim trailing empties
while (kept.length && kept[kept.length - 1].trim() === '') kept.pop();

const out = kept.slice();
if (!out.some(l => /^FoldersCount=/.test(l))) {
  const j = out.findIndex(l => /^FiltersCount=/.test(l));
  out.splice(j >= 0 ? j + 1 : 1, 0, 'FoldersCount=' + dirs.length);
}
// ensure UnitCount exists right after HintUpdate in [Project]
if (!out.some(l => /^UnitCount=/.test(l))) {
  const i = out.findIndex(l => /^HintUpdate=/.test(l));
  out.splice(i >= 0 ? i + 1 : 1, 0, 'UnitCount=' + files.length);
}
files.forEach((f, i) => {
  const dir = path.posix.dirname(f);
  out.push('');
  out.push('[Unit' + (i + 1) + ']');
  out.push('FileName=' + path.posix.basename(f));
  out.push('Folder=' + (dir === '.' ? '' : dir));
});
out.push('');
out.push('[Folders]');
dirs.forEach((d, i) => out.push('Folders' + (i + 1) + '=' + d));

// fix FoldersCount and type/include lines
const text = out.map(l => {
  return l
    .replace(/^FoldersCount=.*/, 'FoldersCount=' + dirs.length)
    .replace(/^name=.*/, 'name=' + name)
    .replace(/^UnitCount=.*/, 'UnitCount=' + files.length)
    .replace(/^filename=.*/, 'filename=' + name + '.lxp')
    .replace(/^DefinedSymbols=.*/, 'DefinedSymbols=' + defines)
    .replace(/^IncludePaths=.*/, 'IncludePaths=' + includes)
    .replace(/^SearchPaths=.*/, 'SearchPaths=' + includes);
}).join('\r\n') + '\r\n';

fs.writeFileSync(lxpPath, text);
console.log(`${name}: ${files.length} units, ${dirs.length} folders`);
