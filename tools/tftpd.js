const dgram = require("dgram");
const fs = require("fs");
const path = require("path");
const ROOT = "C:/Users/mmirz/fs/tftp";
const LOG = "C:/Users/mmirz/fs/tftpd.log";
const log = (m) => fs.appendFileSync(LOG, "[" + new Date().toLocaleTimeString() + "] " + m + "\n");

const sock = dgram.createSocket("udp4");
const sessions = new Map();

function sendNext(s) {
  const chunk = s.data.subarray(s.offset, s.offset + s.blksize);
  const blk = (s.blk + 1) & 0xffff;
  const hdr = Buffer.alloc(4);
  hdr[1] = 3; hdr[2] = (blk >> 8) & 0xff; hdr[3] = blk & 0xff;
  sock.send(Buffer.concat([hdr, chunk]), s.rinfo.port, s.rinfo.address);
  s.blk = blk;
  s.offset += chunk.length;
  if (chunk.length < s.blksize) {
    s.done = true;
    log("DONE " + s.name + " (" + s.data.length + " bytes) -> " + s.rinfo.address);
  } else if (s.offset % 2097152 < s.blksize) {
    log("  " + s.name + " " + s.offset + "/" + s.data.length);
  }
}

sock.on("message", (msg, rinfo) => {
  const op = msg.readUInt16BE(0);
  const key = rinfo.address + ":" + rinfo.port;

  if (op === 1) {
    const parts = msg.subarray(2).toString("latin1").split("\0").filter(Boolean);
    const fname = parts[0];
    const opts = {};
    for (let i = 2; i < parts.length; i += 2) opts[parts[i].toLowerCase()] = parts[i + 1];
    const fp = path.join(ROOT, path.basename(fname));
    if (!fs.existsSync(fp)) {
      const e = Buffer.alloc(13); e[1] = 5; e[2] = 0; e[3] = 1; e.write("notfound", 4);
      sock.send(e, rinfo.port, rinfo.address); log("RRQ " + fname + " NOT FOUND"); return;
    }
    const data = fs.readFileSync(fp);
    const blksize = Math.min(parseInt(opts.blksize || "512", 10) || 512, 65464);
    log("RRQ " + fname + " raw=[" + parts.join("|") + "] from " + key);
    const s = { data, blksize, blk: 0, offset: 0, rinfo, name: fname, done: false };
    sessions.set(key, s);

    let hasOpts = false, oackParts = [];
    if (opts.blksize) { hasOpts = true; oackParts.push("blksize", String(blksize)); }
    // tsize intentionally omitted: PMON rejects OACK containing it

    if (hasOpts) {
      const body = Buffer.from(oackParts.join("\0") + "\0", "latin1");
      const hdr = Buffer.alloc(2); hdr[1] = 6;
      sock.send(Buffer.concat([hdr, body]), rinfo.port, rinfo.address);
      log("OACK -> " + key + " [" + oackParts.join(",") + "]");
    } else {
      sendNext(s);
    }
    return;
  }

  if (op === 4) {
    const s = sessions.get(key);
    if (!s || s.done) return;
    if (msg.readUInt16BE(2) === s.blk) sendNext(s);
  }
});

setInterval(() => {
  for (const s of sessions.values()) {
    if (s.done || !s.blk) continue;
    const start = s.offset - s.blksize;
    if (start < 0) continue;
    const chunk = s.data.subarray(start, s.offset);
    const hdr = Buffer.alloc(4);
    hdr[1] = 3; hdr[2] = (s.blk >> 8) & 0xff; hdr[3] = s.blk & 0xff;
    sock.send(Buffer.concat([hdr, chunk]), s.rinfo.port, s.rinfo.address);
  }
}, 2000);

sock.bind(69, "0.0.0.0", () => log("TFTP ready (OACK + monotonic offsets) on UDP 69"));
