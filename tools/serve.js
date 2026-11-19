const root = "C:/Users/mmirz/OneDrive/Documents/WXWork/1688854477779076/Cache/File/2026-11/loongson_linux_file/loongson_linux_file";
const upDir = "C:/Users/mmirz/fs/uploaded";
Bun.serve({
  port: 8000,
  hostname: "0.0.0.0",
  async fetch(req) {
    const u = new URL(req.url);
    const name = decodeURIComponent(u.pathname.slice(1)) || "index.txt";
    if (req.method === "POST" || req.method === "PUT") {
      const fname = name.replace(/^upload\/?/, "") || "upload.bin";
      const buf = Buffer.from(await req.arrayBuffer());
      await Bun.write(upDir + "/" + fname, buf);
      console.log(`[${new Date().toLocaleTimeString()}] UPLOADED ${fname} (${buf.length} bytes)`);
      return new Response("ok " + buf.length + "\n");
    }
    const f = Bun.file(root + "/" + name);
    if (await f.exists()) {
      console.log(`[${new Date().toLocaleTimeString()}] SERVED ${name} (${f.size} bytes)`);
      return new Response(f, { headers: { "Content-Type": "application/octet-stream" } });
    }
    console.log(`[${new Date().toLocaleTimeString()}] 404 ${name}`);
    return new Response("not found", { status: 404 });
  },
});
console.log("serving+upload on http://0.0.0.0:8000/  root=" + root);
