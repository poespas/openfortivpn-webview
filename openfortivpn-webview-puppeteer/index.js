const yargs = require('yargs/yargs')
const { hideBin } = require('yargs/helpers')
const { Console } = require('console');
const puppeteer = require('puppeteer');

const parser = yargs(hideBin(process.argv))
  .command('[host:port]', 'Connect to the given host')
  .option('username', {
    describe: 'The username to use for authentication.',
    type: "string",
  })
  .option('password', {
    describe: 'The password to use for authentication.',
    type: "string",
  })
  .option('otp', {
    describe: 'The OTP to use for authentication.',
    type: "string",
  })
  .help();
const argv = parser.parse();

if (argv._.length == 0 && !argv.url) {
  parser.showHelp()
  process.exit(1);
}

const urlBuilder = () => {
  if (argv.url) {
    return argv.url;
  } else {
    const realm = argv.realm ? `?realm=${argv.realm}` : '';
    return `https://${argv._[0]}/remote/saml/start${realm}`;
  }
};

// start puppeteer
(async () => {
  const browser = await puppeteer.launch({ headless: false });
  const page = await browser.newPage();
  await page.goto(urlBuilder());

  // wait for the page to load
  await page.waitForSelector('input[name="identifier"]');
  await page.type('input[name="identifier"]', argv.username);
  await page.click('input[type="submit"]');
  await page.waitForSelector('input[name="credentials.passcode"]');
  await page.type('input[name="credentials.passcode"]', argv.password);
  await page.click('input[type="submit"]');

  await new Promise(resolve => setTimeout(resolve, 1000));

  // check if page has challenge-authenticator--google_otp
  const hasChallenge = await page.evaluate(() => {
    return document.querySelector('.challenge-authenticator--google_otp') !== null;
  });

  if (hasChallenge) {
    await page.waitForSelector('input[name="credentials.passcode"]');
    await page.type('input[name="credentials.passcode"]', argv.otp);
    await page.click('input[type="submit"]');
  }

  await new Promise(resolve => setTimeout(resolve, 2000));

  const url = await page.url();

  if (url.includes(argv._[0])) {
    const cookie = await page.cookies();
    console.log(cookie.map(c => `${c.name}=${c.value}`).join('\n'));
  }

  await browser.close();
})();
