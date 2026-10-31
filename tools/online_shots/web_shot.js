// Screenshots web pages: node web_shot.js <url> <out.png> [<url> <out.png> ...]
const { chromium } = require('playwright');

(async () => {
  const browser = await chromium.launch();
  const page = await browser.newPage({ viewport: { width: 900, height: 700 } });
  const args = process.argv.slice(2);
  for (let i = 0; i + 1 < args.length; i += 2) {
    await page.goto(args[i]);
    await page.waitForTimeout(1500);
    await page.screenshot({ path: args[i + 1], fullPage: true });
  }
  await browser.close();
})();
