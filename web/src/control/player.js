import {Visual} from '../present/visual.js';

export class Player
{
    constructor()
    {
        const canvas = document.querySelector("#viewCanvas");
        this.visual = new Visual(canvas);
    }
}