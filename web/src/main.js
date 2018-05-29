import {Game} from './game.js';

main();

function main()
{
    let game = new Game();
    game.run();

    // console.log("main loaded");
    document.querySelector("#welcomeScreen").classList.add("hidden");
}

